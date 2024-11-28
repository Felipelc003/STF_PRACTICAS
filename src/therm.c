#include "therm.h"
#include <math.h>
#include <esp_log.h>

// Definir una etiqueta para los logs
static const char *TAG = "THERM";

// Función para inicializar el termistor y configurar el ADC
esp_err_t therm_init(therm_t *thermistor, adc_channel_t channel) {

    // Configuración del ADC
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,                // Usar ADC1
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,   // Fuente de reloj por defecto
    };

    adc_oneshot_new_unit(&unit_cfg, &thermistor->adc_hdlr);

    // Configurar el canal del ADC
    thermistor->adc_channel = channel;

    return ESP_OK;
}

// Lee el valor en BINARIO del ADC
uint16_t therm_read_lsb(therm_t thermistor) {
    int raw_value = 0;
    adc_oneshot_read(thermistor.adc_hdlr, thermistor.adc_channel, &raw_value);
    return (uint16_t)raw_value;
}

// Convierte el valor en BINARIO a VOLTAJE
float _therm_lsb2v(uint16_t lsb) {
    float voltage = (lsb / 4095.0) * 3.3;  // Resolución de 12 bits y rango de 3.3V
    return voltage;
}

// Convierte el VOLTAJE a TEMPERATURA (en grados Celsius)
float _therm_v2t(float v) {
    float resistance = (SERIES_RESISTANCE * v) / (3.3 - v);  // Resistencia calculada
    float temperature = 1.0 / (log(resistance / NOMINAL_RESISTANCE) / BETA_COEFFICIENT + 1.0 / NOMINAL_TEMPERATURE);
    temperature -= 273.15;  // Convertir de Kelvin a Celsius
    return temperature;
}

// Convierte el valor BINARIO del ACD a TEMPERATURA
float therm_read_t(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);//Lee el BINARIO del ADC
    float voltage = _therm_lsb2v(lsb);//Convierte el Bianrio a Voltaje
    return _therm_v2t(voltage);//Convierte el Voltje a Temperatura
}

// OBTIENE el voltaje del termistor
float therm_read_v(therm_t thermistor) {
    uint16_t lsb = therm_read_lsb(thermistor);
    return _therm_lsb2v(lsb);
}







