/* Motor.c
 * This file contains code related to the motors, including
 * initialization and moving/turning.
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

/* Includes (with #include) code by Daniel and Jonathan Valvano licensed under the Simplified BSD license, reproduced below.
 *
Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
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
#include "Motor.h"
#include "Clock.h"

#define PWM_SCALE 8 // used in the motor functions to make PWM smoother (not so clicky). Higher scale = increased smoothness but higher minimum duty.

// Initializes the 6 GPIO lines for the motors and puts driver to sleep.
void Motor_InitSimple()
{
    // Initialize P1.6 and P1.7 and make them outputs
    P5->SEL0 &= ~0x30; // GPIOs
    P5->SEL1 &= ~0x30;
    P2->SEL0 &= ~0xC0; // GPIOs
    P2->SEL1 &= ~0xC0;
    P3->SEL0 &= ~0xC0; // GPIOs
    P3->SEL1 &= ~0xC0;

    P5->DIR |= 0x30; // outputs
    P2->DIR |= 0xC0; // outputs
    P3->DIR |= 0xC0; // outputs

    P5->OUT &= ~0x30; // direction = 0 (forward)
    P2->OUT &= ~0xC0; // PWM = 0
    P3->OUT &= ~0xC0; // sleep = 0
}

// Stops both motors, puts driver to sleep.
void Motor_StopSimple(void)
{
    P5->OUT &= ~0x30; // direction = 0 (forward)
    P2->OUT &= ~0xC0; // PWM = 0
    P3->OUT &= ~0xC0; // low current sleep mode
}

// Drives both motors forward at duty (100 to 9900).
// Runs for time duration (units=10ms), and then stops.
// Returns after time*10ms.
void Motor_ForwardSimple(uint16_t duty, uint32_t time)
{
    P5->OUT &= ~0x30; // both motors forward
    P3->OUT |= 0xC0; // don't sleep
    uint16_t duty1 = 10000 - duty;

    int i;
    int max = 0.23 * time * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn the motor on
        Clock_Delay1us(1.0f * duty / 10000 * 48 / PWM_SCALE * 1000);
        P2->OUT &= ~0xC0; // turn the motor off
        Clock_Delay1us(1.0f * duty1 / 10000 * 48 / PWM_SCALE * 1000);
    }
}

// Drives both motors backward at duty (100 to 9900).
// Runs for time duration (units=10ms), and then stops.
// Returns after time*10ms.
void Motor_BackwardSimple(uint16_t duty, uint32_t time)
{
    P5->OUT |= 0x30; // both motors backward
    P3->OUT |= 0xC0; // don't sleep
    uint16_t duty1 = 10000 - duty;

    int i;
    int max = 0.23 * time * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn the motor on
        Clock_Delay1us(1.0f * duty / 10000 * 48 / PWM_SCALE * 1000);
        P2->OUT &= ~0xC0; // turn the motor off
        Clock_Delay1us(1.0f * duty1 / 10000 * 48 / PWM_SCALE * 1000);
    }
}

// Drives just the left motor forward at duty (100 to 9900).
// Right motor is stopped (sleeping).
// Runs for time duration (units=10ms), and then stops.
// Returns after time*10ms.
void Motor_LeftSimple(uint16_t duty, uint32_t time)
{
    P5->OUT &= ~0b10000; // left motor forward
    P3->OUT |= 0x80; // left motor don't sleep
    P2->OUT &= ~0x40; // stop right motor
    uint16_t duty1 = 10000 - duty;

    int i;
    int max = 0.23 * time * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0x80; // turn the motor on
        Clock_Delay1us(1.0f * duty / 10000 * 48 / PWM_SCALE * 1000);
        P2->OUT &= ~0x80; // turn the motor off
        Clock_Delay1us(1.0f * duty1 / 10000 * 48 / PWM_SCALE * 1000);
    }
}

// Drives just the right motor forward at duty (100 to 9900).
// Left motor is stopped (sleeping).
// Runs for time duration (units=10ms), and then stops.
// Returns after time*10ms.
void Motor_RightSimple(uint16_t duty, uint32_t time)
{
    P5->OUT &= ~0b1000; // right motor forward
    P3->OUT |= 0x40; // right motor don't sleep
    P2->OUT &= ~0x80; // stop left motor
    uint16_t duty1 = 10000 - duty;

    int i;
    int max = 0.23 * time * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0x40; // turn the motor on
        Clock_Delay1us(1.0f * duty / 10000 * 48 / PWM_SCALE * 1000);
        P2->OUT &= ~0x40; // turn the motor off
        Clock_Delay1us(1.0f * duty1 / 10000 * 48 / PWM_SCALE * 1000);
    }
}

// Spins the robot to the right by 90 degrees using both wheels.
void Motor_SpinRight90()
{
    P5->OUT &= ~0b100000; // left motor forward
    P5->OUT |= 0b10000; // right motor backward
    P3->OUT |= 0xC0; // both motors don't sleep

    int i;
    int max = 0.23 * 47 * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn both motors on
        Clock_Delay1ms(1.0f * 3500 / 10000 * 48 / PWM_SCALE);
        P2->OUT &= ~0xC0; // turn both motors off
        Clock_Delay1ms(1.0f * 6400 / 10000 * 48 / PWM_SCALE);
    }
}
void Motor_SpinRight45()
{
    P5->OUT &= ~0b100000; // left motor forward
    P5->OUT |= 0b10000; // right motor backward
    P3->OUT |= 0xC0; // both motors don't sleep

    int i;
    int max = 0.23 * 47 * PWM_SCALE/2;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn both motors on
        Clock_Delay1ms(1.0f * 3500 / 10000 * 48 / PWM_SCALE);
        P2->OUT &= ~0xC0; // turn both motors off
        Clock_Delay1ms(1.0f * 6400 / 10000 * 48 / PWM_SCALE);
    }
}

// Spins the robot to the left by 90 degrees using both wheels.
void Motor_SpinLeft90()
{
    P5->OUT &= ~0b10000; // right motor forward
    P5->OUT |= 0b100000; // left motor backward
    P3->OUT |= 0xC0; // both motors don't sleep

    int i;
    int max = 0.23 * 47 * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn both motors on
        Clock_Delay1ms(1.0f * 3500 / 10000 * 48 / PWM_SCALE);
        P2->OUT &= ~0xC0; // turn both motors off
        Clock_Delay1ms(1.0f * 6400 / 10000 * 48 / PWM_SCALE);
    }
}

void Motor_SpinLeft45()
{
    P5->OUT &= ~0b10000; // right motor forward
    P5->OUT |= 0b100000; // left motor backward
    P3->OUT |= 0xC0; // both motors don't sleep

    int i;
    int max = 0.23 * 47 * PWM_SCALE / 2;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn both motors on
        Clock_Delay1ms(1.0f * 3500 / 10000 * 48 / PWM_SCALE);
        P2->OUT &= ~0xC0; // turn both motors off
        Clock_Delay1ms(1.0f * 6400 / 10000 * 48 / PWM_SCALE);
    }
}

// Spins the robot 180 degrees.
void Motor_Spin180()
{
    P5->OUT &= ~0b100000; // left motor forward
    P5->OUT |= 0b10000; // right motor backward
    P3->OUT |= 0xC0; // both motors don't sleep

    int i;
    int max = 0.27 * 95 * PWM_SCALE;
    for (i = 0; i < max; i++) // run max times
    {
        P2->OUT |= 0xC0; // turn both motors on
        Clock_Delay1ms(1.0f * 3500 / 10000 * 48 / PWM_SCALE);
        P2->OUT &= ~0xC0; // turn both motors off
        Clock_Delay1ms(1.0f * 6400 / 10000 * 48 / PWM_SCALE);
    }
}
