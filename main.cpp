#include "mbed.h"

#include "State.h"
#include "Thermistor.h"
#include "LightSensor.h"

#include <iostream>

class Estados {
private:
    double umbral_bajo;
    double umbral_alto;
    State estado;

public:
    // Constructor
    Estados(double umbral_bajo, double umbral_alto) 
        : umbral_bajo(umbral_bajo), umbral_alto(umbral_alto), estado(State::NORMAL) {}

    // Método para actualizar el estado
    State actualizar(double valor_actual) {
        if (valor_actual >= umbral_bajo && valor_actual <= umbral_alto) {
            estado = State::NORMAL;
        } 
        else if ((valor_actual > umbral_alto && valor_actual <= umbral_alto + 3) || 
                 (valor_actual < umbral_bajo && valor_actual >= umbral_bajo - 3)) {
            estado = State::WARNING;
        } 
        else {
            estado = State::ALARM;
        }
        
        return estado;
    }

    // Método opcional para consultar el estado actual
    State getEstado() const {
        return estado;
    }
};

/* PERIFERICOS */
Thermistor thermistor(A0);
LightSensor lightsensor(A1);
PwmOut led(D3);

// main() runs in its own thread in the OS
int main()
{
    float temperature, light_current, light_compens;
    Estados state_machine(10,20);
    State state;

    led.period_ms(10);
    
    while ( true ) {
        
        temperature = thermistor.celsius();

        state = state_machine.actualizar(temperature);

        light_current = lightsensor.readPercentage();
        light_compens = 100.0f - light_current;

        led.write(light_compens/100.0f);

        printf("Temp: %.2f ºC | Light: %.2f l -> %.2f l\n", temperature, light_current, light_compens);

        ThisThread::sleep_for(500ms);
    }
}
