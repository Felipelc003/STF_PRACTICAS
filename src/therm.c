#include <math.h>
#include <esp_log.h>
#include "therm.h"

// Definir una etiqueta para los logs
static const char *TAG = "THERM";

// Función para inicializar el termistor y configurar el ADC
esp_err_t therm_init(therm_t *thermistor, adc_channel_t channel) {
    if (!thermistor) {
        ESP_LOGE(TAG, "Termistor nulo");
        return ESP_ERR_INVALID_ARG;
    }

    // Configuración del ADC
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,                // Usar ADC1
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,   // Fuente de reloj por defecto
    };
    esp_err_t ret = adc_oneshot_new_unit(&unit_cfg, &thermistor->adc_hdlr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al inicializar el ADC: %s", esp_err_to_name(ret));
        return ret;  // Retorna el error si no se pudo inicializar el ADC
    }

    // Configurar el canal del ADC
    thermistor->adc_channel = channel;

    ESP_LOGI(TAG, "Termistor configurado en el canal %d", channel);
    return ESP_OK;
}

// Lee la temperatura del termistor (en grados Celsius)
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
        return 0;  // Retorna 0 en caso de error
    }
    ESP_LOGD(TAG, "Valor ADC crudo leído: %d", raw_value);
    return (uint16_t)raw_value;
}

// Convierte el voltaje a temperatura (en grados Celsius)
float _therm_v2t(float v) {
    if (v <= 0) {
        ESP_LOGE(TAG, "Voltaje inválido: %f", v);
        return NAN;
    }

    float resistance = (SERIES_RESISTANCE * v) / (3.3 - v);  // Resistencia calculada
    float temperature = 1.0 / (log(resistance / NOMINAL_RESISTANCE) / BETA_COEFFICIENT + 1.0 / NOMINAL_TEMPERATURE);
    temperature -= 273.15;  // Convertir de Kelvin a Celsius

    ESP_LOGD(TAG, "Resistencia calculada: %f ohms, Temperatura: %f °C", resistance, temperature);
    return temperature;
}

// Convierte un valor LSB a voltaje
float _therm_lsb2v(uint16_t lsb) {
    float voltage = (lsb / 4095.0) * 3.3;  // Resolución de 12 bits y rango de 3.3V
    ESP_LOGD(TAG, "Voltaje calculado a partir de LSB: %f V", voltage);
    return voltage;
}

// Ejemplo de uso del módulo en el código principal
void therm_example_usage(void) {
    // Configuración del ADC
    adc_oneshot_unit_handle_t adc_hdlr;
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_hdlr));

    // Configuración del termistor
    therm_t t1;
    ESP_ERROR_CHECK(therm_init(&t1, ADC_CHANNEL_6));

    // Lecturas
    float temperature = therm_read_t(t1);
    float voltage = therm_read_v(t1);
    uint16_t lsb = therm_read_lsb(t1);

    // Salida de resultados
    ESP_LOGI(TAG, "Temperatura: %.2f °C", temperature);
    ESP_LOGI(TAG, "Voltaje: %.2f V", voltage);
    ESP_LOGI(TAG, "Valor crudo LSB: %d", lsb);
}
