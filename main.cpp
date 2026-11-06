#include "mbed.h"

#include "StateMachine.h"
#include "Thermistor.h"
#include "LightSensor.h"

#include <cstdio>
#include <iostream>

/* DEFINICIONES */
#define NORMAL_STATE_RANGE  4 // 4 ºC
#define WARNING_STATE_RANGE 3 // 3 ºC

/* DISPOSITIVOS E/S */
Thermistor  thermistor(A0);
LightSensor lightsensor(A1);
AnalogIn    potentiometer(A3);
PwmOut      led(D3);
InterruptIn button(D4);
PwmOut      buzzer(D5);
InterruptIn user_button(PC_13);

/* VARIABLES GLOBALES */
State       state_current;
Ticker      buzzer_ticker_warning, buzzer_ticker_alarm;

/* REGISTROS DE CONTROL */
unsigned char buzzer_reg = 0;
unsigned char button_reg = 0;

void RSI_user_button () {
    state_current = State::ACKNOWLEDGED;
}

void RSI_button () {
    button_reg = ! button_reg;
}

void RSI_toggle_buzzer_warning () {
    if ( state_current == State::WARNING ) {
        buzzer.write( buzzer_reg & ( 1 << 0 ) ? 0.5f : 0.0f );
        buzzer_reg ^= ( 1 << 0 );
    } else if ( state_current == State::NORMAL ) {
        buzzer.write( 0.0f );
    }
}

void RSI_toggle_buzzer_alarm () {
    if ( state_current == State::ALARM ) {
        buzzer.write( buzzer_reg & ( 1 << 1 ) ? 0.5f : 0.0f );
        buzzer_reg ^= ( 1 << 1 );
    } else if ( state_current == State::NORMAL ) {
        buzzer.write( 0.0f );
    }
}

int main()
{
    /* VARIABLES LOCALES */
    float temperature;
    float light_current, light_compens;
    float potentiometer_value, potentiometer_value_prev;
    float threshold_lower = 20, threshold_upper = 24;

    StateMachine state_machine(threshold_lower,threshold_upper, WARNING_STATE_RANGE);

    /* INICIALIZAR ELEMENTOS PWM */
    led.period_ms(10);
    buzzer.period_us(500);

    /* CONFIGURAR INTERRUPCIONES BOTONES */
    user_button.fall(&RSI_user_button);
    button.fall(&RSI_button);

    /* INICIALIZAR TICKERS PARA BUZZER */
    buzzer_ticker_warning.attach(&RSI_toggle_buzzer_warning, 100ms);
    buzzer_ticker_alarm.attach(&RSI_toggle_buzzer_alarm, 50ms);
   
    while ( true ) {
        
        temperature = thermistor.celsius();

        state_current = state_machine.update(temperature);

        light_current = lightsensor.readPercentage();
        light_compens = 100.0f - light_current;

        led.write(light_compens/100.0f);

        potentiometer_value = potentiometer.read() * 100 - 40;   

        if ( button_reg ) {
            // Límite Superior
            if ( potentiometer_value >= threshold_lower + NORMAL_STATE_RANGE ) {
                threshold_upper = potentiometer_value;
            } else {
                threshold_upper = threshold_lower + NORMAL_STATE_RANGE;
            }
        } else {
            // Límite Inferior
            if ( potentiometer_value <= threshold_upper - NORMAL_STATE_RANGE ) {
                threshold_lower = potentiometer_value;
            } else {
                threshold_lower = threshold_upper - NORMAL_STATE_RANGE;
            }
        }

        potentiometer_value_prev = potentiometer_value;

        state_machine.setLowerThreshold(threshold_lower);
        state_machine.setUpperThreshold(threshold_upper);

        printf("State: %d | Temp: %.2f ºC | Light: %.2f l -> %.2f l | Pot: %.2f | EDITANDO: %s | Limites (%.2f, %.2f)\n", 
            state_current, 
            temperature,
            light_current, 
            light_compens, 
            potentiometer_value,
            button_reg ? "SUP" : "INF",
            threshold_lower,
            threshold_upper);

        ThisThread::sleep_for(100ms);
    }
}
