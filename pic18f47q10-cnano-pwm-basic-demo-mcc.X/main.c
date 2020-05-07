/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.79.0
        Device            :  PIC18F47Q10
        Driver Version    :  2.00
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"

/* Button is active in low-logic level  */
#define BUTTON_PRESSED                  false
/* Long-press button is 1.5 sec (maximum 2.55 sec) */
#define BUTTON_LONG_PRESS_THRESHOLD     1500
/* Time is expressed in ms */
#define BUTTON_TIME_STEP                  10

#define TIMER_PRESCALER                   32
#define FREQUENCY_TO_PR_CONVERT(F)      (uint8_t)(((_XTAL_FREQ)/ \
                                        (4*(F))/(TIMER_PRESCALER))-1)
#define DUTYCYCLE_TO_CCPR_CONVERT(D,F)  (uint16_t)((float)(D)*(((_XTAL_FREQ)/ \
                                        (F)/(TIMER_PRESCALER))-1)/100.0)

/* Hz */
#define FREQUENCY_MAX                      4
#define FREQUENCY_MIN                      1
#define FREQUENCY_STEP                     1
/* percents */
#define DUTYCYCLE_MAX                     75
#define DUTYCYCLE_MIN                     25
#define DUTYCYCLE_STEP                    25


typedef enum{
    BT_NOCHANGE,
    BT_SHORT_PRESS,
    BT_LONG_PRESS
} button_t;


static button_t ButtonCheck(void)
{
    button_t result = BT_NOCHANGE;
    uint8_t counter = 0;
    static bool old_button_state = !BUTTON_PRESSED;
    bool button_state = Button_GetValue();
    /* detecting only the button-press event */
    if( (button_state == BUTTON_PRESSED) && (old_button_state != BUTTON_PRESSED) )
    {
        /*  wait for debouncing time */
        __delay_ms(BUTTON_TIME_STEP);
        while( (Button_GetValue() == BUTTON_PRESSED) && \
                (counter < (BUTTON_LONG_PRESS_THRESHOLD/BUTTON_TIME_STEP)) )
        {
            /* then stay in the loop until either */
            /* is button released or long-press encountered*/
            counter++;
            __delay_ms(BUTTON_TIME_STEP);
        }
        if(counter)
        {
            result = BT_SHORT_PRESS;
            if(counter >= (BUTTON_LONG_PRESS_THRESHOLD/BUTTON_TIME_STEP))
                result = BT_LONG_PRESS;
        }
    }
    old_button_state = button_state;
    return result;
}

void main(void)
{
    uint8_t frequency  = FREQUENCY_MIN;  /* expressed in Hz */
    uint8_t duty_cycle = DUTYCYCLE_MIN;  /* expressed in percents */
    /* Initialize the device */
    SYSTEM_Initialize();
    PWM1_LoadDutyValue(DUTYCYCLE_TO_CCPR_CONVERT(duty_cycle, frequency));
    TMR2_LoadPeriodRegister(FREQUENCY_TO_PR_CONVERT(frequency));

    while (1)
    {
        button_t button_status = ButtonCheck();
        if(button_status == BT_SHORT_PRESS)
        {
            /* when short button press is detected, change the duty cycle (25%, 50% or 75%) */
            duty_cycle += DUTYCYCLE_STEP;
            if(duty_cycle > DUTYCYCLE_MAX)
                duty_cycle = DUTYCYCLE_MIN;

            /* update the duty cycle */
            PWM1_LoadDutyValue(DUTYCYCLE_TO_CCPR_CONVERT(duty_cycle, frequency));
        }
        else if(button_status == BT_LONG_PRESS)
        {
            /* when long button press is detected, change the frequency (1, 2, 3 or 4 Hz) */
            frequency += FREQUENCY_STEP;
            if(frequency > FREQUENCY_MAX)
                frequency = FREQUENCY_MIN;

            /* update the duty cycle and frequency */
            PWM1_LoadDutyValue(DUTYCYCLE_TO_CCPR_CONVERT(duty_cycle, frequency));
            TMR2_LoadPeriodRegister(FREQUENCY_TO_PR_CONVERT(frequency));
        }
    }
}


