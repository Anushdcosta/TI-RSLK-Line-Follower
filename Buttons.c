
#include "msp.h"
#include "Buttons.h"
#include "SysTick.h"
#include "Globals.c"
#include "GenInterrupts.h"
#include "TimerAs.h"

// Initializes the left and right buttons to send an interrupt when one is pressed
void OnBoardButtons_Init()
{
    // Port 1.1 = left button
    // Port 1.4 = right button
    // Activate on falling edge (on touch).

    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12; // GPIO
    P1->DIR &= ~0x12; // inputs
    P1->REN |= 0x12; // activate pull resistors
    P1->OUT |= 0x12; // make the pull resistors pull-up

    // Make the buttons send interrupts
    P1->IES &= ~0x12; // rising edge
    P1->IFG &= ~0x12; // clear interrupt flag for the buttons
    P1->IE |= 0x12; // arm interrupt on button
    NVIC->IP[8] = (NVIC->IP[8] & 0x00FFFFFF) | 0x40000000; // priority 2
    NVIC->ISER[1] |= 0x00000008; // enable interrupt #35
}

// Handles a button being pressed.
void PORT1_IRQHandler(void)
{
    uint8_t iFlags = P1->IFG; // store the interrupt flag to know which button was pressed
    P1->IFG &= ~0x12; // clear the switch's interrupt flag (acknowledge interrupt)
    if (iFlags & 0x02) // if the left button was pressed
    {
        if ((state == STOPPED) || (state == WIN)) // if the robot was stopped when the button was pressed
        {
            // Restart the maze solving as if it's a new maze
            state = RUNNING; // set the state to RUNNING to run the maze solver again (not show the solution)
        }
        else // if the robot was solving the maze (RUNNING) or showing the solution (SOLUTIONING)
        {
            state = STOPPED; // set the robot's state to stopped
        }
    }
    else // the right button was pressed
    {
        if ((state == STOPPED) || (state == WIN)) // if the robot was stopped when the button was pressed
        {
            // Show how to solve the maze, which may or may not have been fully solved
            state = RUNNING; // set the state to RUNNING to run the maze solver again (not show the solution)

        }
        else if (state == SOLUTIONING) // if the robot was showing the solution when the button was pressed
        {
            state = STOPPED; // set the robot's state to stopped
        }
        else // state == RUNNING
        {
            return; // the robot shouldn't react to the right button being pressed if it's currently solving the maze
        }
    }
    TimerA0_Stop(); // stop Timer A0 so the top LEDs stop flashing if they are flashing
    SysTick_EnableInterrupt(); // re-enable the SysTick interrupt if it was disabled
    SysTick_Restart(); // reload SysTick with 0.2 second timer
    EnableInterrupts(); // enable interrupts in general, though we shouldn't be here if they're disabled
}
