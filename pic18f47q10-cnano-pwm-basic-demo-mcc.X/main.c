 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
© [2023] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
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