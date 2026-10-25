#pragma once

#include "mbed.h"

class LightSensor {
    private:
        AnalogIn pin;

    public:
        LightSensor ( PinName pin );

        // Devuelve el valor de la luz en porcentaje (0.0% a 100.0%)
        float readPercentage();
};