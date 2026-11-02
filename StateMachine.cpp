#include "StateMachine.h"
// Constructor
StateMachine::StateMachine(double umbral_bajo, double umbral_alto) 
    : umbral_bajo(umbral_bajo), umbral_alto(umbral_alto), estado(State::NORMAL) {}

// Método para actualizar el estado
State StateMachine::update(double valor_actual) {
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

// Método para consultar el estado actual
State StateMachine::getState() const {
    return estado;
}