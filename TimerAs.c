/* TimerAs.c
 * This file contains code related to Timer A0, including
 * initialization, interrupt handling, starting, and stopping.
 * Timer A0 is used to blink the LEDs on the breadboard on
 * top of the robot.
 */

/* Licensed under Simplified BSD license by Christopher Andrews.
 *
Copyright 2019 Christopher Andrews

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Includes (with #include) code licensed under the BSD 3-clause license, reproduced below.
 *
* Copyright (C) 2012 - 2017 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*  Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*
*  Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the
*  distribution.
*
*  Neither the name of Texas Instruments Incorporated nor the names of
*  its contributors may be used to endorse or promote products derived
*  from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "msp.h"
#include "TimerAs.h"

void (*PeriodicTask)(void); // the function to call when Timer A0 sends an interrupt
uint8_t numTimes; // the number of times to call PeriodicTask upon interrupt
uint16_t count; // the number of times the interrupt has been called. Used to only call the function every X interrupts

// Initializes Timer A0, but doesn't start it yet.
void TimerA0_Init()
{
    TIMER_A0->CTL &= ~0x0030; // stop Timer A0
    TIMER_A0->CTL = 0x0202; // compare causes interrupts, no I/O
    TIMER_A0->CCTL[0] = 0x0010; // compare
    //TIMER_A0->CCR[0] = (period-1); // compare match value
    TIMER_A0->EX0 = 0x3; // configure for clock input divider /8
    NVIC->IP[2] = (NVIC->IP[2] & 0xFFFFFF00) | 0x00000040; // priority 2
    NVIC->ISER[0] = 0x00000100; // enable interrupt 8 in NVIC
    //TIMER_A0->CTL |= 0x0014; // reset and start Timer A0
}

// Starts Timer A0 and sets it to:
// Interrupt every 50 "period"s.
// Interrupt only "times" number of times.
// Call "task" when it interrupts.
//
// task: Input. The function to call when Timer A0 interrupts.
// period: Input. The time between each function call from Timer A0. Gets multiplied by 50 by only calling the function every 50 interrupts.
// times: Input. The number of times to call that function before disabling Timer A0.
void TimerA0_Start(void(*task)(void), uint16_t period, uint8_t times)
{
    PeriodicTask = task; // store the function to call when Timer A0 interrupts
    numTimes = times; // store the number of times to call the function
    TIMER_A0->CTL &= ~0x0030; // stop Timer A0
    TIMER_A0->CCR[0] = period - 1; // the time between each interrupt
    count = 0; // reset the number of interrupts received from Timer A0
    TIMER_A0->CTL |= 0x0014; // reset and start Timer A0
}

// Handles when Timer A0 interrupts.
void TA0_0_IRQHandler()
{
    TIMER_A0->CCTL[0] &= ~0x0001; // acknowledge interrupt 0
    count++; // increment the number of interrupts received from Timer A0
    if (count == 50) // if we've reached the required number of interrupts to call a function
    {
        count = 0; // reset the dividing counter
        (*PeriodicTask)(); // call the function necessary when Timer A0 interrupts
        numTimes--; // decrement the number of times to call PeriodicTask

        if (numTimes == 0) // if we shouldn't call the function anymore
        {
            TIMER_A0->CTL &= ~0x0030; // stop Timer A0
        }
        else
        {
            TIMER_A0->CTL |= 0x0014; // reset and start Timer A0
        }
    }
}

// Forces Timer A0 to stop.
inline void TimerA0_Stop()
{
    TIMER_A0->CTL &= ~0x0030; // stop Timer A0
}
