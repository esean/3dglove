// **** Include libraries here ****
// Standard libraries

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "HardwareDefs.h"
#include "Adc.h"
#include "Buttons.h"
#include "Leds.h"
#include "Oled.h"
// **** Set macros and preprocessor directives ****

// **** Declare any datatypes here ****

// **** Define global, module-level, or external variables here ****
char string[30] = "INIT";
static int sw = 0;
static int changed = 0;
static int btn = 0;
// **** Declare function prototypes ****

// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 20 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
#pragma config FPLLIDIV   = DIV_2     // Set the PLL input divider to 2
#pragma config FPLLMUL    = MUL_20    // Set the PLL multiplier to 20
#pragma config FPLLODIV   = DIV_1     // Don't modify the PLL output.
#pragma config FNOSC      = PRIPLL    // Set the primary oscillator to internal RC w/ PLL
#pragma config FSOSCEN    = OFF       // Disable the secondary oscillator
#pragma config IESO       = OFF       // Internal/External Switch O
#pragma config POSCMOD    = XT        // Primary Oscillator Configuration
#pragma config OSCIOFNC   = OFF       // Disable clock signal output
#pragma config FPBDIV     = DIV_4     // Set the peripheral clock to 1/4 system clock
#pragma config FCKSM      = CSECMD    // Clock Switching and Monitor Selection
#pragma config WDTPS      = PS1       // Specify the watchdog timer interval (unused)
#pragma config FWDTEN     = OFF       // Disable the watchdog timer
#pragma config ICESEL     = ICS_PGx2  // Allow for debugging with the Uno32
#pragma config PWP        = OFF       // Keep the program flash writeable
#pragma config BWP        = OFF       // Keep the boot flash writeable
#pragma config CP         = OFF       // Disable code protect

int main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
    // Given the options, this function will change the flash wait states, RAM
    // wait state and enable prefetch cache but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above..
    SYSTEMConfig(F_SYS, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // Auto-configure the PIC32 for optimum performance at the specified operating frequency.
    SYSTEMConfigPerformance(F_SYS);

    // osc source, PLL multipler value, PLL postscaler , RC divisor
    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_1, OSC_FRC_POST_1);

    // Configure the PB bus to run at 1/4th the CPU frequency, so 20MHz.
    OSCSetPBDIV(OSC_PB_DIV_4);

    // Enable multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
    INTEnableInterrupts();

    // Set up the UART peripheral so we can send serial data.
    UARTConfigure(UART_USED, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART_USED, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART_USED, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART_USED, F_PB, UART_BAUD_RATE);
    UARTEnable(UART_USED, UART_ENABLE_FLAGS(UART_TX));

    // And configure printf/scanf to use UART_USED if it's different from the default of UART2.
    if (UART_USED == UART1) {
        __XC_UART = 1;
    }

    // Configure Timer 1 using PBCLK as input. This default period will make the LEDs blink at a
    // pretty reasonable rate to start.
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_8, 0xFFFF);

    // Set up the timer interrupt with a priority of 4.
    INTClearFlag(INT_T1);
    INTSetVectorPriority(INT_TIMER_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T1, INT_ENABLED);

    /***************************************************************************************************
     * Your code goes in between this comment and the following one with asterisks.
     **************************************************************************************************/

    //disables all A/D pins for a clean start
    AD1PCFG = 0xffff;
//    TRISFbits.TRISF0 = 0;
//    LATFbits.LATF5 = 1;
//    LATFbits.LATF4 = 1;
//    LATFbits.LATF3 = 1;
//    LATFbits.LATF2 = 1;
//    LATFbits.LATF1 = 1;
//    LATFbits.LATF0 = 1;


    // INITS
    OledInit();
    ButtonsInit();
    AdcInit();
    LEDS_INIT();


    LEDS_SET(0);
    OledDrawString(string);
    OledUpdate();

    int adc;
    double voltlvl = 0;

    while (1) {
        // Check for switch or button or potentiometer change
        if (AdcChanged() || (changed != 0)) {
            LEDS_SET(btn);

            // adc value to voltage
            adc = AdcRead();
            voltlvl = ((double) adc / 1023)*100;

            // update screen
            sprintf(string, "Button: %2d\nVoltLvl: %5.01f%%\nSwitch: %2d", btn, voltlvl, sw);
            OledDrawString(string);
            OledUpdate();
        }
    }
    /***************************************************************************************************
     * Your code goes in between this comment and the preceding one with asterisks
     **************************************************************************************************/

    while (1);
}

/**
 * This is the interrupt for the Timer1 peripheral. During each call it increments a counter (the
 * value member of a module-level TimerResult struct). This counter is then checked against the
 * binary values of the four switches on the I/O Shield (where SW1 has a value of 1, SW2 has a value
 * of 2, etc.). If the current counter is greater than this switch value, then the event member of a
 * module-level TimerResult struct is set to true and the value member is cleared.
 */
void __ISR(_TIMER_1_VECTOR, ipl4) Timer1Handler(void) {
    changed = 0;
    // SWITCHES
    int temp = SWITCH_STATES();
    if (temp != sw) {
        sw = temp;
        changed = 1;
    }

    // BUTTONS
    int check = BUTTON_STATES();
    if (check != 0) {
        btn = check;
        changed = 1;
    } else if (check == 0) {
        btn = 0;
        changed = 1;
    }
    // Clear Interupt
    IFS0CLR = 1 << 4;
}
