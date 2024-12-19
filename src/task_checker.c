#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <esp_log.h>
#include <math.h>
#include <string.h>

static const char* TAG = "STF_P1:task_checker";

SYSTEM_TASK(TASK_CHECKER) {
    TASK_BEGIN();
    ESP_LOGI(TAG, "Task Checker running");

    // Argumentos de la tarea
    RingbufHandle_t* checker_rbuf = ((RingbufHandle_t*)TASK_ARGS);
    RingbufHandle_t* monitor_rbuf = ((RingbufHandle_t*)TASK_ARGS);

    // Variables locales
    size_t length;
    void* ptr;
    float temp1, temp2, desviacion;

    TASK_LOOP() {
        // Recibir T1 y T2 del Sensor
        ptr = xRingbufferReceive(*checker_rbuf, &length, pdMS_TO_TICKS(1000));
        if (ptr != NULL) {
            
            temp1 = *((float*)ptr);
            temp2 = *(((float*)ptr) + 1);

            // Calcular desviación
            desviacion = 100 * (temp1 - temp2) / temp1;

            // Enviar desviación al Monitor
            message_monitor_t message = { .id_source = 2, .data = desviacion}; 
            xRingbufferSendAcquire(*monitor_rbuf, &message, sizeof(message_monitor_t), pdMS_TO_TICKS(100));
                
            
        }
    }

    TASK_END();
}
