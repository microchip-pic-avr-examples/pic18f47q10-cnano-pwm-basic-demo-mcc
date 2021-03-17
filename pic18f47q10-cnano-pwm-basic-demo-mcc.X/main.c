/*
Copyright (c) [2012-2020] Microchip Technology Inc.  

    All rights reserved.

    You are permitted to use the accompanying software and its derivatives 
    with Microchip products. See the Microchip license agreement accompanying 
    this software, if any, for additional info regarding your rights and 
    obligations.
    
    MICROCHIP SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT 
    WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
    LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT 
    AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP OR ITS
    LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT 
    LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE 
    THEORY FOR ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT 
    LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, 
    OR OTHER SIMILAR COSTS. 
    
    To the fullest extend allowed by law, Microchip and its licensors 
    liability will not exceed the amount of fees, if any, that you paid 
    directly to Microchip to use this software. 
    
    THIRD PARTY SOFTWARE:  Notwithstanding anything to the contrary, any 
    third party software accompanying this software is subject to the terms 
    and conditions of the third party's license agreement.  To the extent 
    required by third party licenses covering such third party software, 
    the terms of such license will apply in lieu of the terms provided in 
    this notice or applicable license.  To the extent the terms of such 
    third party licenses prohibit any of the restrictions described here, 
    such restrictions will not apply to such third party software.
*/
#include "mcc_generated_files/system/system.h"

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
    Timer2_PeriodCountSet(FREQUENCY_TO_PR_CONVERT(frequency));

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
            Timer2_PeriodCountSet(FREQUENCY_TO_PR_CONVERT(frequency));
        }
    }
}