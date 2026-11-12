#include "mbed.h"

#include "StateMachine.h"
#include "Thermistor.h"
#include "LightSensor.h"
#include "Grove_LCD_RGB_Backlight.h"
#include <cstdio>
#include <iostream>

/* DEFINICIONES */
#define NORMAL_STATE_RANGE  4 // 4 ºC
#define WARNING_STATE_RANGE 3 // 3 ºC
#define INITIAL_THRESHOLD_LOWER 20
#define INITIAL_THRESHOLD_UPPER 26

/* DISPOSITIVOS E/S */
Thermistor  thermistor(A0, 5);
LightSensor lightsensor(A1);
AnalogIn    potentiometer(A3);
PwmOut      led(D3);
InterruptIn   button(D4);
PwmOut      buzzer(D5);
InterruptIn user_button(PC_13);

/* VARIABLES GLOBALES */
State           state_current;
Ticker          buzzer_ticker_warning, buzzer_ticker_alarm;
StateMachine    state_machine(INITIAL_THRESHOLD_LOWER,INITIAL_THRESHOLD_UPPER, WARNING_STATE_RANGE);
float           potentiometer_value, threshold_lower, threshold_upper;
bool            editing;

/* REGISTROS DE CONTROL */
unsigned char buzzer_reg = 0;
volatile bool button_pressed_flag = false;

void RSI_user_button () {
    state_current = State::ACKNOWLEDGED;
}

void RSI_button() {    
    button_pressed_flag = true; // Solo avisamos de que ocurrió la interrupción
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

    /* INICIALIZAR ELEMENTOS PWM */
    led.period_ms(10);
    buzzer.period_us(500);

    /* CONFIGURAR INTERRUPCIONES BOTONES */
    user_button.fall(&RSI_user_button);
    button.fall(&RSI_button);
   
    /* INICIALIZAR TICKERS PARA BUZZER */
    buzzer_ticker_warning.attach(&RSI_toggle_buzzer_warning, 100ms);
    buzzer_ticker_alarm.attach(&RSI_toggle_buzzer_alarm, 50ms);

    /* PANTALLA I2C */
    Grove_LCD_RGB_Backlight display(D14,D15);

    while ( true ) {

        temperature   = thermistor.celsius();

        state_current = state_machine.update(temperature);

        light_current = lightsensor.readPercentage();
        light_compens = 100.0f - light_current;

        led.write(light_compens/100.0f);

        potentiometer_value = potentiometer.read() * 100 - 40;

        // Determinamos si editamos el umbral inferior o superior
        threshold_lower = state_machine.getLowerThreshold();
        threshold_upper = state_machine.getUpperThreshold();
        editing = ( abs(potentiometer_value - threshold_lower) < abs(potentiometer_value - threshold_upper) ) ? 0 : 1;
        
        if ( button_pressed_flag ) {
            button_pressed_flag = false;

            if ( editing ) {
                if ( potentiometer_value >= threshold_lower + NORMAL_STATE_RANGE ) {
                    threshold_upper = potentiometer_value;
                } else {
                    threshold_upper = threshold_lower + NORMAL_STATE_RANGE;
                }
                state_machine.setUpperThreshold(threshold_upper);
            } else {
                if ( potentiometer_value <= threshold_upper - NORMAL_STATE_RANGE ) {
                    threshold_lower = potentiometer_value;
                } else {
                    threshold_lower = threshold_upper - NORMAL_STATE_RANGE;
                }
                state_machine.setLowerThreshold(threshold_lower);
            }
        }

        printf("State: %d | Temp: %.2f ºC | Light: %.2f l (%.2f) -> %.2f l | ( %.2f %s %.2f %s %.2f ) -> %d %d\n", 
            state_current, 
            temperature,
            light_current, 
            light_compens,
            light_compens/100.0f, 
            state_machine.getLowerThreshold(),
            (! editing) ? "<" : "|",
            potentiometer_value,
            (editing) ? ">" : "|",
            state_machine.getUpperThreshold(),
            editing,
            button.read());

        ThisThread::sleep_for(100ms);
    }
}
