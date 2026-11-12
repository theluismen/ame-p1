#include "StateMachine.h"

// Constructor
StateMachine::StateMachine(float umbral_bajo, float umbral_alto, float warning_state_range) 
    : umbral_bajo(umbral_bajo), umbral_alto(umbral_alto), estado(State::NORMAL), warning_state_range(warning_state_range) {}

// Método para actualizar el estado
State StateMachine::update(float valor_actual) {
    if (valor_actual >= umbral_bajo && valor_actual <= umbral_alto) {
        estado = State::NORMAL;
    } 
    else if ((valor_actual > umbral_alto && valor_actual <= umbral_alto + warning_state_range) || 
             (valor_actual < umbral_bajo && valor_actual >= umbral_bajo - warning_state_range)) {
        estado = State::WARNING;
    } 
    else {
        estado = State::ALARM;
    }
    
    return estado;
}

// Método para consultar el estado actual
State StateMachine::getState() const {
    return estado;
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