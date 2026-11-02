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

State       state_current, state_previous;
Ticker      buzzer_ticker;
bool        buzzer_state = 0;

void RSI_boton () {
    state_current = State::ACKNOWLEDGED;
}

void toggle_buzzer () {
    buzzer_state = ! buzzer_state;
    buzzer.write(buzzer_state ? 0.5f : 0.0f);
}


int main()
{
    float temperature, light_current, light_compens;
    StateMachine state_machine(20,24);

    led.period_ms(10);
    buzzer.period_us(500);

    button.fall(&RSI_boton);

    while ( true ) {
        
        temperature = thermistor.celsius();

        printf("State: %d\n", state_current);

        state_current = state_machine.update(temperature);

        light_current = lightsensor.readPercentage();
        light_compens = 100.0f - light_current;

        switch ( state_current ) {
            case State::NORMAL:
                buzzer_ticker.detach();
                break;
            case State::WARNING:
                if ( state_previous != State::WARNING)
                    buzzer_ticker.attach(&toggle_buzzer, 100ms);
                break;
            case State::ALARM:
                if ( state_previous != State::ALARM)
                    buzzer_ticker.attach(&toggle_buzzer, 50ms);
                break;
            default:
                break;
        }

        led.write(light_compens/100.0f);

        printf("Temp: %.2f ºC | Light: %.2f l -> %.2f l\n", temperature, light_current, light_compens);

        state_previous = state_current;

        ThisThread::sleep_for(100ms);
    }
}
