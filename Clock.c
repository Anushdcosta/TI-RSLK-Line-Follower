#include "msp.h"
#include "Clock.h"

uint32_t ClockFrequency = 3000000; // clock speed, cycles/second. Default is 3MHz

uint32_t waitTime; // represents the total amount of time spent waiting on something in the Clock_Init48MHz() function. Should be reset to 0 before using
uint32_t IFlags = 0; // non-zero if clock transition is invalid

// Attempts to initialize the clock to be 48MHz. Leaves it at the previous value if unsuccessful.
void Clock_Init48MHz(void)
{
    // Wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    waitTime = 0; // reset the wait count
    while (PCM->CTL1 & 0x00000100) // while the clock is not ready
    {
        waitTime = waitTime + 1; // add 1 to the total waiting time
        if (waitTime >= 100000) // if we have reached the max time we're willing to wait
        {
            return; // time out error
        }
    }

    // Request power "Active Mode LDO VCORE1" to support the 48 MHz frequency (using pg 436 of Technical Reference Manual)
    PCM->CTL0 = (PCM->CTL0 & ~0xFFFF000F) | // clear PCMKEY bit field and AMR bit field
            0x695A0000 | // write the proper PCM key to unlock write access
            0x00000001; // request power active mode LDO VCORE1

    // Check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
    if (PCM->IFG & 0x00000004) // if the transition to AM_LDO_VCORE1 is invalid
    {
        IFlags = PCM->IFG; // bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
        PCM->CLRIFG = 0x00000004; // clear the transition invalid flag
        // to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
        // or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int32kHz() or Clock_InitLowPower() has been called
        return;
    }

    // Wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LDO VCORE1
    waitTime = 0; // reset the wait count
    while ((PCM->CTL0 & 0x00003F00) != 0x00000100) // while we haven't switched to AM_LDO_VCORE1 yet
    {
        waitTime = waitTime + 1; // add 1 to the total waiting time
        if (waitTime >= 500000) // if we have reached the max time we're willing to wait
        {
            return; // time out error
        }
    }

    // Wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    waitTime = 0; // reset the wait count
    while (PCM->CTL1 & 0x00000100) // while PCM isn't idle yet
    {
        waitTime = waitTime + 1; // add 1 to the total waiting time
        if (waitTime >= 100000) // if we have reached the max time we're willing to wait
        {
            return; // time out error
        }
    }

    // Initialize PJ.3 and PJ.2 and make them HFXT (PJ.3 built-in 48 MHz crystal out; PJ.2 built-in 48 MHz crystal in)
    PJ->SEL0 |= 0x0C;
    PJ->SEL1 &= ~0x0C; // configure built-in 48 MHz crystal for HFXT operation
    CS->KEY = 0x695A; // unlock CS module for register access
    CS->CTL2 = (CS->CTL2 & ~0x00700000) | // clear HFXTFREQ bit field
            0x00600000 | // configure for 48 MHz external crystal
            0x00010000 | // HFXT oscillator drive selection for crystals >4 MHz
            0x01000000; // enable HFXT
    CS->CTL2 &= ~0x02000000; // disable high-frequency crystal bypass

    // Wait for the HFXT clock to stabilize
    waitTime = 0; // reset the wait count
    while (CS->IFG & 0x00000002) // while the clock isn't stable yet
    {
        CS->CLRIFG = 0x00000002; // clear the HFXT oscillator interrupt flag
        waitTime = waitTime + 1; // add 1 to the total waiting time
        if (waitTime > 100000) // if we have reached the max time we're willing to wait
        {
            return; // time out error
        }
    }

    // Configure for 2 wait states (minimum for 48 MHz operation) for flash Bank 0
    FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~0x0000F000)
            | FLCTL_BANK0_RDCTL_WAIT_2;

    // Configure for 2 wait states (minimum for 48 MHz operation) for flash Bank 1
    FLCTL->BANK1_RDCTL = (FLCTL->BANK1_RDCTL & ~0x0000F000)
            | FLCTL_BANK1_RDCTL_WAIT_2;

    CS->CTL1 = 0x20000000 | // configure for SMCLK divider /4
            0x00100000 | // configure for HSMCLK divider /2
            0x00000200 | // configure for ACLK sourced from REFOCLK
            0x00000050 | // configure for SMCLK and HSMCLK sourced from HFXTCLK
            0x00000005; // configure for MCLK sourced from HFXTCLK
    CS->KEY = 0; // lock CS module from unintended access
    ClockFrequency = 48000000; // set the clock frequency for doing math later on (usually inside delay functions)
}

// delay function
// which delays about 6*ulCount cycles
// ulCount=8000 => 1ms = (8000 loops)*(6 cycles/loop)*(20.83 ns/cycle)
// Code Composer Studio Code
void delay(unsigned long ulCount)
{
    __asm ("pdloop:  subs r0, #1\n"
           "         bne  pdloop\n");
}

// Simple delay function which delays about n milliseconds at 48MHz.
void Clock_Delay1ms(uint32_t n)
{
    while (n) // while n > 0 (runs n times)
    {
        delay(ClockFrequency / 9162); // 1 msec, tuned at 48 MHz
        n--; // decrement n so while-loop runs n times
    }
}

// Simple delay function which delays about n microseconds at 48MHz.
void Clock_Delay1us(uint32_t n)
{
    n = (382*n) / 100;; // 1 us, tuned at 48 MHz
    while (n) // while n > 0 (run n times)
    {
        n--; // decrement n so while-loop runs n times
    }
}
