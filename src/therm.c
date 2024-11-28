#include <math.h>
#include <esp_log.h>
#include "therm.h"



// Definir una etiqueta para los logs
static const char *TAG = "THERM";

// Configura el termistor con un manejador ADC y un canal específico
esp_err_t therm_config(therm_t* thermistor, adc_oneshot_unit_handle_t adc, adc_channel_t channel) {
    if (!thermistor) return ESP_ERR_INVALID_ARG;

    thermistor->adc_hdlr = adc;
    thermistor->adc_channel = channel;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_DB_11,  // Rango de 0 a 3.3V
        .bitwidth = ADC_BITWIDTH_12  // Resolución de 12 bits
    };
    return adc_oneshot_config_channel(adc, channel, &channel_cfg);
}

// Lee el valor en BINARIO del ADC
uint16_t therm_read_lsb(therm_t thermistor) {
    int raw_value = 0;
    esp_err_t ret = adc_oneshot_read(thermistor.adc_hdlr, thermistor.adc_channel, &raw_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer el ADC: %s", esp_err_to_name(ret));
        return 0;
    }
    return (uint16_t)raw_value;
}

// Convierte el valor en BINARIO a VOLTAJE
float _therm_lsb2v(uint16_t lsb) {
    float voltage = (lsb / 4095.0) * 3.3;  // Resolución de 12 bits y rango de 3.3V
    return voltage;
}

// Convierte el VOLTAJE a TEMPERATURA (en grados Celsius)
float _therm_v2t(float v) {
    float resistance = (SERIES_RESISTANCE * v) / (VOLTAJE_REFERENCIA - v);  // Resistencia calculada
    float temperature = 1.0 / (log(resistance / NOMINAL_RESISTANCE) / BETA_COEFFICIENT + 1.0 / NOMINAL_TEMPERATURE);
    temperature -= 273.15;  // Convertir de Kelvin a Celsius
    return temperature;
}

// Convierte el valor BINARIO del ACD a TEMPERATURA
float therm_read_t(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor); // Lee el BINARIO del ADC
    float voltage = _therm_lsb2v(lsb);         // Convierte el BINARIO a Voltaje
    return _therm_v2t(voltage);                // Convierte el Voltaje a Temperatura
}

// OBTIENE el voltaje del termistor
float therm_read_v(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);
    return _therm_lsb2v(lsb);
}
