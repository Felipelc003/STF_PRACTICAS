/**********************************************************************
* FILENAME : task_sensor.c       
*
* DESCRIPTION : 
* Esta tarea lee periódicamente la temperatura de un termistor, 
* calcula su valor en grados Celsius y lo coloca en un buffer cíclico
* para ser utilizado por otra tarea.
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
#include <math.h>
#include <sys/time.h>
#include <string.h>

// freertos
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// espidf
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>

// propias
#include "config.h"
#include "therm.h"  // Nuevo módulo termistor

static const char *TAG = "STF_P1:task_sensor";

// Esta tarea temporiza la lectura de un termistor. 
// Se usa un semáforo global para coordinar la periodicidad de las lecturas.
static SemaphoreHandle_t semSample = NULL;
static void tmrSampleCallback(void* arg)
{
    xSemaphoreGive(semSample);
}

// Tarea SENSOR
SYSTEM_TASK(TASK_SENSOR)
{	
    TASK_BEGIN();
    ESP_LOGI(TAG,"Task Sensor running");

    // Recibe los argumentos de configuración de la tarea y los desempaqueta
    task_sensor_args_t* ptr_args = (task_sensor_args_t*) TASK_ARGS;
    RingbufHandle_t* rbuf = ptr_args->rbuf; 
    uint8_t frequency = ptr_args->freq;
    uint64_t period_us = 1000000 / frequency;

    // Estructura de datos para configurar el termistor (ADC y canal se configuran en therm_init)
    therm_t termistor;
    therm_init(&termistor, ADC_CHANNEL_6);  // Ahora pasamos el canal directamente

    // Inicializa el semáforo (la estructura del manejador se definió globalmente)
    semSample = xSemaphoreCreateBinary();

    // Crea y establece una estructura de configuración para el temporizador
    const esp_timer_create_args_t tmrSampleArgs = {
        .callback = &tmrSampleCallback,
        .name = "Timer Configuration"
    };

    // Lanza el temporizador, con el periodo de muestreo recibido como parámetro
    esp_timer_handle_t tmrSample;
    ESP_ERROR_CHECK(esp_timer_create(&tmrSampleArgs, &tmrSample));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tmrSample, period_us));

    // Variables para reutilizar en el bucle
    void *ptr;
    float temperature;

    // Loop
    TASK_LOOP()
    {
        // Se bloquea a la espera del semáforo. Si el periodo se retrasa un 20%, el sistema se reinicia.
        if(xSemaphoreTake(semSample, ((1000/frequency)*1.2)/portTICK_PERIOD_MS))
        {	
            // Lee la temperatura del termistor. Se obtiene el valor en grados centígrados
            temperature = therm_read_t(termistor);
            ESP_LOGI(TAG, "Valor medido (pre buffer): %f", temperature);

            // Uso del buffer cíclico entre la tarea monitor y sensor
            // Pide al RingBuffer espacio para escribir un float. 
            if (xRingbufferSendAcquire(*rbuf, &ptr, sizeof(float), pdMS_TO_TICKS(100)) != pdTRUE)
            {
                // Si falla la reserva de memoria, notifica la pérdida del dato.
                ESP_LOGI(TAG,"Buffer lleno. Espacio disponible: %d", xRingbufferGetCurFreeSize(*rbuf));
            }
            else 
            {
                // Si xRingbufferSendAcquire tiene éxito, se escribe el número de bytes solicitados
                memcpy(ptr, &temperature, sizeof(float));
                // Se notifica que la escritura ha completado. 
                xRingbufferSendComplete(*rbuf, ptr);
            }
        }
        else
        {
            ESP_LOGI(TAG,"Watchdog (soft) failed");
            esp_restart();
        }
    }
    
    ESP_LOGI(TAG,"Deteniendo la tarea...");
    // Detención controlada de las estructuras que ha levantado la tarea
    ESP_ERROR_CHECK(esp_timer_stop(tmrSample));
    ESP_ERROR_CHECK(esp_timer_delete(tmrSample));
    TASK_END();
}
