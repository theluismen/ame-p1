#ifndef ESTADOS_H
#define ESTADOS_H

#include "State.h"

class StateMachine {
private:
    double umbral_bajo;
    double umbral_alto;
    State estado;

public:
    // Constructor
    StateMachine(double umbral_bajo, double umbral_alto);

    // Método para actualizar el estado
    State update(double valor_actual);

    // Método para consultar el estado actual
    State getState() const;
};

#endif