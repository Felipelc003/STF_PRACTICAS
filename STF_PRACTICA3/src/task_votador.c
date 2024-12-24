#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <esp_log.h>
#include <string.h>

static const char* TAG = "STF_P1:task_votador";

SYSTEM_TASK(TASK_VOTADOR) {
    TASK_BEGIN();
    ESP_LOGI(TAG, "Task Votador running");

    task_votador_args_t* ptr_args = (task_votador_args_t*)TASK_ARGS;
    RingbufHandle_t* rbuf_sensor = ptr_args->rbuf_sensor;
    RingbufHandle_t* rbuf_monitor = ptr_args->rbuf_monitor;
    uint32_t mask = ptr_args->mask; // Declara y asigna la máscara

    size_t length;
    void *ptr;
    uint16_t raw_values[3];
    uint16_t result;

    TASK_LOOP() {
        // Bloquea en espera de datos del RingBuffer de sensores
        ptr = xRingbufferReceive(*rbuf_sensor, &length, pdMS_TO_TICKS(1000));

        if (ptr != NULL) {
            if (length == sizeof(raw_values)) {
                // Copia los datos en el arreglo de valores crudos
                memcpy(raw_values, ptr, sizeof(raw_values));

            // // Imprime los valores crudos antes de aplicar la máscara (solo para probar que está funcoinado la máscara)
            // ESP_LOGI(TAG, "Valores crudos antes de la máscara: %04X, %04X, %04X", raw_values[0], raw_values[1], raw_values[2]);

                // Aplica la máscara a los valores crudos
                raw_values[0] &= mask;
                raw_values[1] &= mask;
                raw_values[2] &= mask;

            // // Imprime los valores crudos después de aplicar la máscara (solo para probar que está funcoinado la máscara)
            // ESP_LOGI(TAG, "Valores crudos después de la máscara: %04X, %04X, %04X", raw_values[0], raw_values[1], raw_values[2]);

                // Implementa la lógica de votación
                result = (raw_values[0] & raw_values[1]) | (raw_values[0] & raw_values[2]) | (raw_values[1] & raw_values[2]);

                // Enviar el valor resultante al buffer del monitor
                if (xRingbufferSend(*rbuf_monitor, &result, sizeof(result), pdMS_TO_TICKS(100)) != pdTRUE) {
                    ESP_LOGW(TAG, "Buffer lleno. No se puede enviar el valor resultante.");
                } 
               //Había un problema que si se usaba xBufferSendAcquire y xBufferReceive, más adelante al usar vRingbufferReturnItem, se bloqueaba el sistema.
            } else {
                ESP_LOGE(TAG, "Datos inesperados recibidos: %d bytes", length);
            }

            // Devuelve el elemento al RingBuffer
            vRingbufferReturnItem(*rbuf_sensor, ptr);
        } else {
            ESP_LOGW(TAG, "Esperando datos ...");
        }
    }

    ESP_LOGI(TAG, "Deteniendo la tarea ...");
    TASK_END();
}