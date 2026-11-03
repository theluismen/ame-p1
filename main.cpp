#include "mbed.h"

#include "StateMachine.h"
#include "Thermistor.h"
#include "LightSensor.h"

#include <cstdio>
#include <iostream>

/* DISPOSITIVOS E/S */
Thermistor  thermistor(A0);
LightSensor lightsensor(A1);
AnalogIn    potentiometer(A3);
PwmOut      led(D3);
AnalogIn    button(D4);
PwmOut      buzzer(D5);
InterruptIn user_button(PC_13);

/* VARIABLES GLOBALES */
State       state_current;
Ticker      buzzer_ticker_warning, buzzer_ticker_alarm;

unsigned char buzzer_reg = 0;

void RSI_boton () {
    state_current = State::ACKNOWLEDGED;
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
    float potentiometer_value;

    StateMachine state_machine(20,24);

    /* INICIALIZAR ELEMENTOS PWM */
    led.period_ms(10);
    buzzer.period_us(500);

    /* CONFIGURAR INTERRUPCIONES BOTÓN */
    user_button.fall(&RSI_boton);

    /* INICIALIZAR TICKERS PARA BUZZER */
    buzzer_ticker_warning.attach(&RSI_toggle_buzzer_warning, 100ms);
    buzzer_ticker_alarm.attach(&RSI_toggle_buzzer_alarm, 50ms);
   
    while ( true ) {
        
        temperature = thermistor.celsius();

        state_current = state_machine.update(temperature);

        light_current = lightsensor.readPercentage();
        light_compens = 100.0f - light_current;

        led.write(light_compens/100.0f);

        potentiometer_value = potentiometer.read() * 100;

        printf("State: %d | Temp: %.2f ºC | Light: %.2f l -> %.2f l | Pot: %.2f\n", 
            state_current, 
            temperature,
            light_current, 
            light_compens, 
            potentiometer_value);

        ThisThread::sleep_for(100ms);
    }
}
