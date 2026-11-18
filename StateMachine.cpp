#include "StateMachine.h"

// Constructor
StateMachine::StateMachine(float umbral_bajo, float umbral_alto, float warning_state_range) 
    : umbral_bajo(umbral_bajo), umbral_alto(umbral_alto), estado(State::NORMAL), warning_state_range(warning_state_range) {
        estado = State::NORMAL;
        estado_prev = State::NORMAL;
    }

// Método para actualizar el estado
State StateMachine::update(float valor_actual) {
    State estado_calculado;

    if (valor_actual >= umbral_bajo && valor_actual <= umbral_alto) {
        estado_calculado = State::NORMAL;
    } else if ((valor_actual > umbral_alto && valor_actual <= umbral_alto + warning_state_range) || 
             (valor_actual < umbral_bajo && valor_actual >= umbral_bajo - warning_state_range)) {
        estado_calculado = State::WARNING;
    } else {
        estado_calculado = State::ALARM;
    }
    
    if ( estado == State::ACKNOWLEDGED ) {
        if ( estado_calculado != estado_prev ) {
            estado = estado_calculado;
        }
    } else {
        estado = estado_calculado;
    }

    return estado;
}

// Método para consultar el estado actual
State StateMachine::getState() const {
    return estado;
}

void StateMachine::acknowledged() {
    this->estado_prev = estado;
    this->estado = State::ACKNOWLEDGED;
}

// Método para modificar el límite superior
void StateMachine::setUpperThreshold ( float t ) {
    this->umbral_alto = t;
}

// Método para modificar el límite superior
void StateMachine::setLowerThreshold ( float t ) {
    this->umbral_bajo = t;
}

// Método para consultar el límite superior
float StateMachine::getUpperThreshold() {
    return umbral_alto;
}

// Método para consultar el límite inferior
float StateMachine::getLowerThreshold() {
    return umbral_bajo;
}