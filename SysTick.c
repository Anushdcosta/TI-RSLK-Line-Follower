
#include "msp.h"
#include "SysTick.h"
#include "Globals.c"
#include "LineSensor.h"

//#define SysTickInterval 0x00927C00 // 0.2 sec
//#define SysTickInterval 0x00493E00 // 0.1 sec
//#define SysTickInterval 0x00249F00 // 0.05 sec
#define SysTickInterval 0x00124F80 // 0.025 sec

// Initializes SysTick to send an interrupt every SysTickInterval clock cycles, and starts SysTick.
void SysTick_Init(void)
{
    SysTick->LOAD = SysTickInterval; // load with the interval we want
    //SysTick->CTRL = 0x00000005; // enable SysTick with no interrupts
    SysTick->CTRL = 0x7; // enable SysTick with interrupts
    SCB->SHP[11] = 4 << 5; // priority 4
}

// Called every time SysTick sends an interrupt.
void SysTick_Handler()
{
    // SysTick automatically acknowledges (resets) the interrupt flag
    lineSensors = LineSensor_Read(); // read the line sensor
    SysTick_Restart(); // reload SysTick
}

// Disables the interrupt enable flag in SysTick.
inline void SysTick_DisableInterrupt()
{
    SysTick->CTRL = 0x5; // enable SysTick with no interrupts
}

// Sets the interrupt enable flag in SysTick.
inline void SysTick_EnableInterrupt()
{
    SysTick->CTRL = 0x7; // enable SysTick with interrupts
}

// Loads SysTickInterval into SysTick so it can restart.
inline void SysTick_Restart()
{
    SysTick->LOAD = SysTickInterval; // load with the interval we want
}
