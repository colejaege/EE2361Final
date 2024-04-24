/*
 * Date: 4/24/2024
 * Name(s): Cole Jaeger, Dhayalan Balasubarmanian, Luke Tschepen, Nick
 * Project Name: Blood Oxygen Sensor
 * Program Description:provides functionality for interfacing with a 
 * physical button switch in the Blood Oxygen Sensor project. 
 * It includes functions for initializing the button, detecting button presses,
 * and determining the state of the button. The button switch is connected
 * to RB7 (Pin 6) of the microcontroller, and its usage involves capturing
 * button press events and toggling the display of heart rate information
 * References/Code Citations: Arduino Grove RGB file, JHD data sheet, Lab 4
 * 
 */

#include "xc.h"
#include "button_switch.h"

// Global variables
volatile unsigned long int prev_t;
volatile unsigned long int cur_t;
volatile int displayHeartRate = 0;
volatile unsigned long int overflow1 = 0;
volatile int prevState = 0;

/**
 * Initializes the button for usage.
 */
void initButton() {
    // Set RB7 (Pin 6) as input for the button
    TRISBbits.TRISB7 = 1;
    // Enable internal pull-up resistor for RB7
    CNPU2bits.CN23PUE = 1;
    
    // Map Input Capture 1 to RB8 (Pin 7)
    __builtin_write_OSCCONL(OSCCON & 0xbf); // Unlock PPS
    RPINR7bits.IC1R = 7;
    __builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS
    
    // Configure Timer2 for overflow calculation
    T2CON = 0;
    T2CONbits.TCKPS = 0b11; // 1:256 prescaler
    PR2 = 62499; // 1 second
    TMR2 = 0;
    
    // Enable Timer2 interrupt
    _T2IE = 1;
    _T2IF = 0;
    T2CONbits.TON = 1;
    
    // Configure Input Capture 1
    IC1CON = 0;
    IC1CONbits.ICTMR = 1; // Use Timer2
    IC1CONbits.ICM = 1; // Capture mode every edge
    _IC1IF = 0;
    _IC1IE = 1;
    _IC1IP = 2;
}

/**
 * Timer2 interrupt service routine.
 */
void __attribute__((__interrupt__,__auto_pav__)) _T2Interrupt(void){
    _T2IF = 0;
    overflow1++;
}

/**
 * Input Capture 1 interrupt service routine.
 */
void __attribute__((__interrupt__, __auto_psv__)) _IC1Interrupt(void){
   _IC1IF = 0; 
   
   // Calculate current time
   unsigned long int cur_t = overflow1 * (uint32_t)(PR2 + 1) + TMR2;
   
   // Check button press duration
   if ((cur_t - prev_t) > 130){
       prev_t = cur_t;
       
       // Toggle button state on press
       if(prevState){ // Previous state: Pressed, current state: Released
           prevState = 0;
       } else { // Previous state: Released, current state: Pressed
           prevState = 1;
           displayHeartRate = !displayHeartRate;
           
       }
   }
}

/**
 * Get the state of the button.
 * @return 1 if heart rate should be displayed, 0 otherwise.
 */
int getButtonState() {
    return displayHeartRate;
}
