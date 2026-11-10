#ifndef ESTADOS_H
#define ESTADOS_H

#include "State.h"

class StateMachine {
private:
    float umbral_bajo;
    float umbral_alto;
    float warning_state_range;
    State estado;


public:
    // Constructor
    StateMachine(float umbral_bajo, float umbral_alto, float warning_state_range);

    // Método para actualizar el estado
    State update(float valor_actual);

    // Método para consultar el estado actual
    State getState() const;

    void setUpperThreshold( float t );

    void setLowerThreshold( float t );
};

#endif