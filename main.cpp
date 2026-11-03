#include "mbed.h"

#include "StateMachine.h"
#include "Thermistor.h"
#include "LightSensor.h"

#include <cstdio>
#include <iostream>

/* PERIFERICOS */
Thermistor  thermistor(A0);
LightSensor lightsensor(A1);
PwmOut      led(D3);
PwmOut      buzzer(D5);
InterruptIn button(D4);

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
    /* VARIABLES GLOBALES */
    float temperature, light_current, light_compens;
    StateMachine state_machine(20,24);

    /* INICIALIZAR ELEMENTOS PWM */
    led.period_ms(10);
    buzzer.period_us(500);

    button.fall(&RSI_boton);

    /* INICIALIZAR TICKERS PARA BUZZER */
    buzzer_ticker_warning.attach(&RSI_toggle_buzzer_warning, 100ms);
    buzzer_ticker_alarm.attach(&RSI_toggle_buzzer_alarm, 50ms);
   
    while ( true ) {
        
        temperature = thermistor.celsius();

        printf("State: %d\n", state_current);

        state_current = state_machine.update(temperature);

        light_current = lightsensor.readPercentage();
        light_compens = 100.0f - light_current;

        led.write(light_compens/100.0f);

        printf("Temp: %.2f ºC | Light: %.2f l -> %.2f l\n", temperature, light_current, light_compens);

        ThisThread::sleep_for(100ms);
    }
}
