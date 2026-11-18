#include "mbed.h"

#include "StateMachine.h"
#include "Thermistor.h"
#include "LightSensor.h"
#include "Grove_LCD_RGB_Backlight.h"
#include <cstdio>
#include <format>
#include <iostream>
#include <string>

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
InterruptIn button(D4);
PwmOut      buzzer(D5);
InterruptIn user_button(PC_13);

/* VARIABLES GLOBALES */
State           state_current;
Ticker          buzzer_ticker_warning, buzzer_ticker_alarm;
StateMachine    state_machine(INITIAL_THRESHOLD_LOWER,INITIAL_THRESHOLD_UPPER, WARNING_STATE_RANGE);
float           potentiometer_value, threshold_lower, threshold_upper;
bool            editing;
float           temperature;
float           light_current, light_compens;

/* PANTALLA I2C */
Grove_LCD_RGB_Backlight display(D14,D15);
char * display_buffer = new char[16];

/* REGISTROS DE CONTROL */
unsigned char buzzer_reg = 0;
volatile bool button_pressed_flag = false;

void RSI_user_button () {
    state_machine.acknowledged();
}

void RSI_button() {    
    button_pressed_flag = true; // Solo avisamos de que ocurrió la interrupción
}

void RSI_toggle_buzzer_warning () {
    State state = state_machine.getState();

    if ( state == State::WARNING ) {
        buzzer.write( buzzer_reg & ( 1 << 0 ) ? 0.5f : 0.0f );
        buzzer_reg ^= ( 1 << 0 );
    } else if ( state == State::NORMAL ) {
        buzzer.write( 0.0f );
    }
}

void RSI_toggle_buzzer_alarm () {
    State state = state_machine.getState();

    if ( state == State::ALARM ) {
        buzzer.write( buzzer_reg & ( 1 << 1 ) ? 0.5f : 0.0f );
        buzzer_reg ^= ( 1 << 1 );
    } else if ( state == State::NORMAL ) {
        buzzer.write( 0.0f );
    }
}

/*
##################
#T:XX.XC    L:XX%#
#(XX|XX|XX) C:XX%#
##################
*/

// TODO
void show_info () {
    /* Show the system data */
    /* Definir búferes independientes para cada línea (asumiendo LCD de 16x2) */
    char display_buffer_row1[17];
    char display_buffer_row2[17];

    /* Formatear la primera fila */
    std::snprintf(display_buffer_row1, sizeof(display_buffer_row1), "T:%4.1fC   L:%3.0f%%", temperature, light_current);

    /* Formatear la segunda fila de forma independiente */
    std::snprintf(display_buffer_row2, sizeof(display_buffer_row2), "(%2.0f%s%2.0f%s%2.0f)C:%3.0f%%", threshold_lower, (! editing) ? "<" : "|", potentiometer_value, (editing) ? ">" : "|", threshold_upper, light_compens);

    /* Mostrar en la pantalla */
    display.locate(0, 0); // Primera fila
    display.print(display_buffer_row1);
    display.locate(0, 1); // Segunda fila
    display.print(display_buffer_row2);

    /* Change color depending on state */
    switch ( state_current ) {
        case State::NORMAL:
            display.setRGB(0x00, 0XFF, 0x00); // GREEN
            break;
        case State::WARNING:
            display.setRGB(0xFF, 0XFF, 0x00); // YELLOW
            break;
        case State::ALARM:
            display.setRGB(0xFF, 0x00, 0x00); // RED
            break;
        case State::ACKNOWLEDGED:
            display.setRGB(0x00, 0x00, 0xFF); // BLUE
            break;
    }
}

void show_boot_sequence() {
    char *boot_msg = "Booting...";

    display.clear();          // Limpia la pantalla antes de empezar
    display.locate(0, 0);     // Coloca el cursor en la columna 0, fila 0
    display.setRGB(0xFF,0xFF,0xFF);

    // Recorre la cadena carácter a carácter hasta llegar al final ('\0')
    for (int i = 0; boot_msg[i] != '\0'; i++) {
        display.writech(boot_msg[i]);     // Envía el carácter individual a la LCD
        ThisThread::sleep_for(150ms); // Pausa de 150 ms entre cada letra
    }
    ThisThread::sleep_for(1s);
}

int main()
{
    /* INICIALIZAR ELEMENTOS PWM */
    led.period_ms(10);
    buzzer.period_us(500);

    /* CONFIGURAR INTERRUPCIONES BOTONES */
    user_button.fall(&RSI_user_button);
    button.fall(&RSI_button);
   
    /* INICIALIZAR TICKERS PARA BUZZER */
    buzzer_ticker_warning.attach(&RSI_toggle_buzzer_warning, 100ms);
    buzzer_ticker_alarm.attach(&RSI_toggle_buzzer_alarm, 50ms);

    show_boot_sequence();

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

        // TODO
        show_info();

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
