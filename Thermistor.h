#pragma once

#include "mbed.h"

class Thermistor {
    private:
        AnalogIn pin;
        float v_cc; // Variable para almacenar el voltaje de alimentación

    public:
        Thermistor( PinName pin, float v_cc = 3.3f );

        ~Thermistor();

        float celsius ();
        float kelvins ();

};