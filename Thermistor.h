#pragma once

#include "mbed.h"

class Thermistor {
    private:
        AnalogIn pin;

    public:
        Thermistor ( PinName pin );

        ~Thermistor();

        float celsius ();
        float kelvins ();

};