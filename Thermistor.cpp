#include "Thermistor.h"
#include <cmath>

Thermistor::Thermistor(PinName pin, float v_cc ) : pin(pin), v_cc(v_cc) {
    
}

float Thermistor::kelvins() {
    float R_serie = 100000.0f; // R_b
    float R_0     = 100000.0f;
    float T_0     = 298.15f;
    float Beta    = 4299.0f;

    float lectura = pin.read(); // [0..1]

    if (lectura <= 0.0f) lectura = 0.00001f;

    float denominador = ( v_cc / ( 3.3f * lectura ) ) - 1.0f;
    if ( denominador <= 0.0f ) denominador = 0.00001f;

    float R_th = R_serie * denominador;

    float T_K = 1.0f / (std::log(R_th / R_0) / Beta + (1.0f / T_0));

    return T_K;
}

float Thermistor::celsius() {
    return kelvins() - 273.15f;
}