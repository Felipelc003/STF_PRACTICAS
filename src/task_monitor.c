// /**********************************************************************
// * FILENAME : task_monitor.c       
// *
// * DESCRIPTION : Monitor de sensores para dos termistores.
// *
// * PUBLIC LICENSE :
// * Este código es de uso público y libre de modificar bajo los términos de la
// * Licencia Pública General GNU (GPL v3) o posterior. Se proporciona "tal cual",
// * sin garantías de ningún tipo.
// *
// * AUTHOR :   Dr. Fernando Leon (fernando.leon@uco.es) University of Cordoba
// ******************************************************************************/

// // libc
// #include <time.h>
// #include <stdio.h>
// #include <sys/time.h>
// #include <string.h>

// // freertos
// #include <freertos/FreeRTOS.h>
// #include <freertos/semphr.h>

// // esp
// #include <esp_system.h>
// #include <esp_event.h>
// #include <esp_log.h>
// #include <esp_timer.h>

// // propias
// #include "config.h"

// static const char *TAG = "STF_P1:task_monitor";

// // Tarea MONITOR
// SYSTEM_TASK(TASK_MONITOR)
// {
//     TASK_BEGIN();
//     ESP_LOGI(TAG, "Task Monitor running");

//     // Recibe los argumentos de configuración de la tarea y los desempaqueta
//     task_monitor_args_t* ptr_args = (task_monitor_args_t*)TASK_ARGS;
//     RingbufHandle_t* rbuf = ptr_args->rbuf; 

//     // Variables para reutilizar en el bucle
//     size_t length;
//     void *ptr;
//     float temperatures[2]; // Para almacenar lecturas de ambos sensores

//     // Loop
//     TASK_LOOP()
//     {
//         // Bloquea en espera de datos del RingBuffer
//         ptr = xRingbufferReceive(*rbuf, &length, pdMS_TO_TICKS(1000));

//         if (ptr != NULL) 
//         {
//             // Verifica que se han recibido dos lecturas de float
//             if (length == sizeof(temperatures)) 
//             {
//                 // Copia los datos en el arreglo de temperaturas
//                 memcpy(temperatures, ptr, sizeof(temperatures));

//                 // Muestra las temperaturas de los sensores
//                 ESP_LOGI(TAG, "Sensor 1: %.2f °C, Sensor 2: %.2f °C", temperatures[0], temperatures[1]);
//             } 
//             else 
//             {
//                 ESP_LOGE(TAG, "Datos inesperados recibidos: %d bytes", length);
//             }

//             // Devuelve el elemento al RingBuffer
//             vRingbufferReturnItem(*rbuf, ptr);
//         } 
//         else 
//         {
//             ESP_LOGW(TAG, "Esperando datos ...");
//         }
//     }

//     ESP_LOGI(TAG, "Deteniendo la tarea ...");
//     TASK_END();
// }



/**********************************************************************
* FILENAME : task_monitor.c       
*
* DESCRIPTION : Monitor de sensores para dos termistores.
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
#include <string.h>
#include <math.h>

// freertos
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// esp
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>

// propias
#include "config.h"

static const char *TAG = "STF_P1:task_monitor";

// Tarea MONITOR
SYSTEM_TASK(TASK_MONITOR)
{
    TASK_BEGIN();
    ESP_LOGI(TAG, "Task Monitor running");

    // Recibe los argumentos de configuración de la tarea y los desempaqueta
    task_monitor_args_t* ptr_args = (task_monitor_args_t*)TASK_ARGS;
    RingbufHandle_t* rbuf = ptr_args->rbuf; 

    // Variables para reutilizar en el bucle
    size_t length;
    void *ptr;
    float t1, t2, deviation_percentage; // Para almacenar lecturas de ambos sensores

    // Loop
    TASK_LOOP()
    {
        // Bloquea en espera de datos del RingBuffer
        ptr = xRingbufferReceive(*rbuf, &length, pdMS_TO_TICKS(1000));

        if (ptr != NULL) 
        {
            // Se asume que los datos recibidos son de dos termistores (pares de float)
            // ptr es un puntero a un array de floats (2 elementos, uno para cada termistor)
            // Primer termistor (original)
			t1 = ((float*)ptr)[0]; // Primer termistor (original)
			t2 = ((float*)ptr)[1]; // Segundo termistor (nuevo)
		
            // Calculamos la desviación en porcentaje entre el primer y segundo termistor
            deviation_percentage = fabs((t1 - t2) / t2) * 100;

            // Imprimimos el valor del primer termistor y la desviación con respecto al segundo
            ESP_LOGI(TAG, "T1:%.5f (%.2f%%)", t1, deviation_percentage);

            // Devolvemos el ítem al RingBuffer
            vRingbufferReturnItem(*rbuf, ptr);
        } 
        else 
        {
            ESP_LOGW(TAG, "Esperando datos ...");
        }
    }

    ESP_LOGI(TAG, "Deteniendo la tarea ...");
    TASK_END();
}