#include "LightSensor.h"

LightSensor::LightSensor(PinName pin) : pin(pin) {
    
}

float LightSensor::readPercentage() {
    float vout = pin.read(); // Lee valor entre 0.0 y 1.0
    
    // Convertimos directamente a porcentaje (0% a 100%)
    float percentage = vout * 100.0f;
    
    // Limitamos por seguridad para que nunca baje de 0 ni suba de 100
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;
    
    return percentage;
}