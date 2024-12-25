/******************************************************************************
* FILENAME : config.h
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

// freertos
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>

// esp
#include <hal/adc_types.h>

// propias
#include "system.h"

// Abstracciones para facilitar la legibilidad
#define CORE0 0
#define CORE1 1

// Configuraciones y constantes

// Nombre y estados de la máquina
#define SYS_NAME "STF P1 System"
enum{
    INIT,
    //SENSOR_LOOP,
    ALL_SENSORS_OK,
    ONE_SENSOR_FAIL,
    CRITICAL_ERROR
};

// Configuración del termistor
// Ahora la configuración del ADC se maneja automáticamente en el módulo term.c, 
// por lo que no es necesario definirla aquí.
#define SERIES_RESISTANCE 10000       // 10K ohms
#define NOMINAL_RESISTANCE 10000      // 10K ohms
#define NOMINAL_TEMPERATURE 298.15    // 25°C en Kelvin
#define BETA_COEFFICIENT 3950         // Constante B (ajustar según el termistor)
#define THERMISTOR_ADC_CHANNEL ADC_CHANNEL_6        // Canal del ADC 1
#define SECOND_THERMISTOR_ADC_CHANNEL ADC_CHANNEL_7 // Canal del ADC 2


// Configuración del buffer cíclico
#define BUFFER_SIZE  2048
#define BUFFER_TYPE  RINGBUF_TYPE_NOSPLIT

// Configuración de las tareas

// SENSOR
// Tarea sensor
SYSTEM_TASK(TASK_SENSOR);
// definición de los argumentos que requiere la tarea
typedef struct 
{
    RingbufHandle_t* rbuf; // puntero al buffer 
    uint8_t freq;          // frecuencia de muestreo
    // ...
}task_sensor_args_t;
// Timeout de la tarea (ver system_task_stop)
#define TASK_SENSOR_TIMEOUT_MS 2000 
// Tamaño de la pila de la tarea
#define TASK_SENSOR_STACK_SIZE 4096


// MONITOR
SYSTEM_TASK(TASK_MONITOR);
// definición de los argumentos que requiere la tarea
typedef struct 
{
    RingbufHandle_t* rbuf; // puntero al buffer 
    // ...
}task_monitor_args_t;
// Timeout de la tarea (ver system_task_stop)
#define TASK_MONITOR_TIMEOUT_MS 2000 
// Tamaño de la pila de la tarea
#define TASK_MONITOR_STACK_SIZE 4096


// VOTADOR
SYSTEM_TASK(TASK_VOTADOR);
// definición de los argumentos que requiere la tarea
typedef struct 
{
    RingbufHandle_t* rbuf_sensor; // puntero al buffer del sensor
    RingbufHandle_t* rbuf_monitor; // puntero al buffer del monitor
    uint32_t mask; // máscara de votación
} task_votador_args_t;
// Timeout de la tarea (ver system_task_stop)
#define TASK_VOTADOR_TIMEOUT_MS 2000 
// Tamaño de la pila de la tarea
#define TASK_VOTADOR_STACK_SIZE 4096

#endif
