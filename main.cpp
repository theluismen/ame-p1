#include "mbed.h"

#include "Thermistor.h"
#include "LightSensor.h"

#include <iostream>

// Definición de los estados posibles
enum class State {
    NORMAL,
    WARNING,
    ALARM,
    ACKNOWLEDGED
};

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

// main() runs in its own thread in the OS
int main()
{
    float temperature;
    Estados state_machine(10,20);
    State state;

    while ( true ) {
        temperature = thermistor.celsius();

        state = state_machine.actualizar(temperature);

        // hacer cosas con el nuevo estado {NORMAL,WARNING,ALARM}

    }
}
