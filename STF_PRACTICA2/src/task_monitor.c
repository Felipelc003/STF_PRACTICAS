/**********************************************************************
* FILENAME : task_monitor.c       
*
* DESCRIPTION : 
*
* PUBLIC LICENSE :
* Este código es de uso público y libre de modificar bajo los términos de la
* Licencia Pública General GNU (GPL v3) o posterior. Se proporciona "tal cual",
* sin garantías de ningún tipo.
*
* AUTHOR :   Dr. Fernando Leon (fernando.leon@uco.es) University of Cordoba
******************************************************************************/

// libc
#include <time.h>
#include <stdio.h>
#include <sys/time.h>

// freerqtos
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// esp
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>

// propias
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <esp_log.h>
#include <string.h>
#include <math.h>

static const char* TAG = "STF_P1:task_monitor";

// Estructura para recibir datos de los sensores
// typedef struct {
//     float temp1; // Temperatura del primer termistor
//     float temp2; // Temperatura del segundo termistor
// } sensor_data_t; //Esta estructura ya no se usa y se puede borrar si se quiere

extern system_t sys_stf_p1;
extern system_task_t task_checker;
extern system_task_t task_monitor;

SYSTEM_TASK(TASK_MONITOR) {
    TASK_BEGIN();
    ESP_LOGI(TAG, "Task Monitor running");

    // Argumentos de la tarea
    task_monitor_args_t* args = (task_monitor_args_t*)TASK_ARGS;
    RingbufHandle_t* rbuf = args->rbuf;

    // Variables locales
    size_t length;
    void* ptr;
    float deviation_percentage = -0.0f;
    float temp1 = -0.0f;

    TASK_LOOP() {
        // Recibe datos del buffer cíclico
		ptr = xRingbufferReceive(*rbuf, &length, pdMS_TO_TICKS(1000));
		if (ptr != NULL) {

            message_monitor_t* message = (message_monitor_t*)ptr;

            if(message->id_source == 1){
                temp1 = message->data;
                //deviation_percentage = -0.0f; // Ya no hay que reiniciar el valor de la desviación, se puede borrar si se quiere
            }else if(message->id_source == 2){
                deviation_percentage = message->data;
            }

            // Devuelves el item al ring buffer
			vRingbufferReturnItem(*rbuf, ptr);

            //temp1 = message->data;

            // printf("id_source: %d\n", message->id_source);
            // printf("data: %f\n", message->data);
            // printf("temp1: %f\n", temp1);
            // printf("deviation_percentage: %f\n\n\n", deviation_percentage);

			// // Procesar el mensaje según su origen
			// if (deviation_percentage <= 0.0) {
			// 	// Mensaje de la tarea Sensor
			// 	ESP_LOGI(TAG, "Termistor 1: %.2f ºC", temp1);

			// } else if (deviation_percentage > 0) {
			// 	// Mensaje de la tarea Comprobador

            //     ESP_LOGI(TAG, "Termistor 1: %.2f ºC", temp1);
			// 	ESP_LOGI(TAG, "Desviación: %.2f %%", deviation_percentage);
			// } //Código antiguo que ya no sirve y se puede borrar si se quiere

            switch(GET_ST_FROM_TASK()) {
                case NORMAL_MODE:
                    ESP_LOGI(TAG, "NORMAL MODE: T = %.2f ºC", temp1);
                    if (deviation_percentage > 10.0 && deviation_percentage < 20.0) {
                        SWITCH_ST_FROM_TASK(DEGRADED_MODE);
                    } else if (deviation_percentage >= 20.0) {
                        SWITCH_ST_FROM_TASK(ERROR);
                    }
                    break;
                case DEGRADED_MODE:
                    ESP_LOGI(TAG, "DEGRADED MODE: T = (%.2f - %.2f) ºC", temp1 - temp1 * deviation_percentage / 100, temp1 + temp1 * deviation_percentage / 100);
                    if (deviation_percentage <= 10.0) {
                        SWITCH_ST_FROM_TASK(NORMAL_MODE);
                    } else if (deviation_percentage >= 20.0) {
                        SWITCH_ST_FROM_TASK(ERROR);
                    }
                    break;
                case ERROR:
                    ESP_LOGI(TAG, "Sensor ERROR. Repare and restart.");
                    system_task_stop(&sys_stf_p1, &task_checker, TASK_CHECKER_TIMEOUT_MS);
                    system_task_stop(&sys_stf_p1, &task_monitor, TASK_MONITOR_TIMEOUT_MS);
                    vTaskDelete(NULL);
                    break;
                default:
                    vTaskDelay(100);
                    break;
            }
		} else {
			ESP_LOGW(TAG, "Esperando datos...");
		}

    }

    TASK_END();
}