#ifndef __THERM_H__
#define __THERM_H__

#include <esp_adc/adc_oneshot.h>
#include <hal/adc_types.h>

// Definición de las constantes del termistor
#define SERIES_RESISTANCE 10000    // 10K ohms
#define NOMINAL_RESISTANCE 10000   // 10K ohms
#define NOMINAL_TEMPERATURE 298.15 // 25°C en Kelvin
#define BETA_COEFFICIENT 3950      // Constante B

// Definición de la estructura para el termistor
typedef struct therm_conf_t
{
    adc_oneshot_unit_handle_t adc_hdlr;  // Manejador del ADC
    adc_channel_t adc_channel;           // Canal del ADC
} therm_t;

// Funciones para configurar y leer del termistor
esp_err_t therm_init(therm_t *thermistor, adc_channel_t channel);
float therm_read_t(therm_t thermistor);
float therm_read_v(therm_t thermistor);
uint16_t therm_read_lsb(therm_t thermistor);

// Funciones privadas de conversión
float _therm_v2t(float v);
float _therm_lsb2v(uint16_t lsb);

#endif
