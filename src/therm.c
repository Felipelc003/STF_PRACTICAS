#include <math.h>
#include <esp_log.h>
#include "therm.h"

// Definir una etiqueta para los logs
static const char *TAG = "THERM";

// Configura un termistor con el manejador del ADC y el canal especificado
esp_err_t therm_config(therm_t *thermistor, adc_oneshot_unit_handle_t adc, adc_channel_t channel) {
    if (!thermistor || !adc) {
        ESP_LOGE(TAG, "Termistor o manejador ADC nulo");
        return ESP_ERR_INVALID_ARG;
    }

    // Asignar configuraciones
    thermistor->adc_hdlr = adc;
    thermistor->adc_channel = channel;

    ESP_LOGI(TAG, "Termistor configurado en el canal %d", channel);
    return ESP_OK;
}

// Lee la temperatura en grados Celsius del termistor
float therm_read_t(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);
    float voltage = _therm_lsb2v(lsb);
    return _therm_v2t(voltage);
}

// Lee el voltaje del termistor
float therm_read_v(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);
    return _therm_lsb2v(lsb);
}

// Lee el valor crudo (LSB) del ADC
uint16_t therm_read_lsb(therm_t thermistor) {
    int raw_value = 0;
    esp_err_t ret = adc_oneshot_read(thermistor.adc_hdlr, thermistor.adc_channel, &raw_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer el ADC: %s", esp_err_to_name(ret));
        return 0; // Retornar 0 en caso de error
    }
    ESP_LOGD(TAG, "Valor ADC crudo leído: %d", raw_value);
    return (uint16_t)raw_value;
}

// Convierte el voltaje en temperatura usando la ecuación del termistor
float _therm_v2t(float v) {
    if (v <= 0) {
        ESP_LOGE(TAG, "Voltaje inválido: %f", v);
        return NAN;
    }

    float resistance = (SERIES_RESISTANCE * v) / (3.3 - v); // Divisor de voltaje
    float temperature = 1.0 / (log(resistance / NOMINAL_RESISTANCE) / BETA_COEFFICIENT + 1.0 / NOMINAL_TEMPERATURE);
    temperature -= 273.15; // Convertir de Kelvin a Celsius

    ESP_LOGD(TAG, "Resistencia calculada: %f ohms, Temperatura: %f °C", resistance, temperature);
    return temperature;
}

// Convierte un valor LSB a voltaje
float _therm_lsb2v(uint16_t lsb) {
    float voltage = (lsb / 4095.0) * 3.3; // Resolución de 12 bits (0-4095) y rango de 3.3V
    ESP_LOGD(TAG, "Voltaje calculado a partir de LSB: %f V", voltage);
    return voltage;
}
