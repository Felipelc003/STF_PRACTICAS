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
typedef struct {
    float temp1; // Temperatura del primer termistor
    float temp2; // Temperatura del segundo termistor
} sensor_data_t;

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
			// Devuelves el item al ring buffer
			vRingbufferReturnItem(*rbuf, ptr);

            if(message->id_source == 1){
                temp1 = message->data;
            }else if(message->id_source == 2){
                deviation_percentage = message->data;
            }

			// Procesar el mensaje según su origen
			if (deviation_percentage <= 0.0) {
				// Mensaje de la tarea Sensor
				ESP_LOGI(TAG, "Termistor 1: %.2f ºC", temp1);

			} else if (deviation_percentage > 0) {
				// Mensaje de la tarea Comprobador

                ESP_LOGI(TAG, "Termistor 1: %.2f ºC", temp1);
				ESP_LOGI(TAG, "Desviación: %.2f %%", deviation_percentage);
			}
		} else {
			ESP_LOGW(TAG, "Esperando datos...");
		}

    }

    TASK_END();
}