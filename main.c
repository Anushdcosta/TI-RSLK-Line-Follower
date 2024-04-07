// Fixed By Umer and Anush

#include "msp.h"
#include "Motor.h"
#include "Clock.h"
#include "GenInterrupts.h"
#include "SysTick.h"
#include "Globals.c"
#include "LineSensor.h"
#include "Buttons.h"
#include "TimerAs.h"

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
}; // used to print out a value in binary

// Called when the program starts.
// Contains the main logic for solving the maze.
void main(void)
    {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // stop watchdog timer

    // Initialize everything
    DisableInterrupts();
    state = STOPPED; // stopped by default
    Clock_Init48MHz(); // run at 48MHz
    Motor_InitSimple(); // initialize the wheel motors
    LineSensor_Init(); // initialize the line/light sensors
    OnBoardButtons_Init(); // initialize the on-board buttons for changing the robot's state (running, stopping, solutioning)
    TimerA0_Init(); // initialize but don't start Timer A0
    SysTick_Init(); // initialize the SysTick timer with interrupts
    EnableInterrupts();

    while (1) // forever
    {   uint8_t sensors = lineSensors; // get the current value of lineSensors in case it is updated mid-loop
        if (sensors == 0x58) {
            sensors = 0x18;
        } else if (sensors == 0x1A){
            sensors = 0x18;
        }
        if ((state == STOPPED)) // if the robot should not be running
        {
            SysTick_DisableInterrupt(); // disable the SysTick interrupt
            lineSensors = 0x18; // so the robot goes forward when enabled instead of moving randomly
            WaitForInterrupt(); // wait for a button press
            continue; // in case a non-button interrupt interrupts here, just go back through the while-loop
        }
        else if (state == RUNNING) // robot should be solving the maze
        {
            if (sensors == 0x00) // if the sensors are all white (off the line)
            {
                lineSensors = LineSensor_Read(); // re-read the line sensor so that we don't use an outdated value
            }
            else if ((sensors == 0xFF) || (sensors == 0x7F) || (sensors == 0xFE) || (sensors == 0x3F)) // if the sensors are all black (T or 4-way intersection)
            {
                Motor_ForwardSimple(MOVE_SPEED*1.15, 4);
            }

            // None of the special if-clauses above were taken, so now just follow the black line.
            else if ((sensors & 0x80) == 0x80) // if sensor 7 is black
            {
                Motor_RightSimple(MOVE_SPEED, 5); // turn left a significant amount
                Motor_ForwardSimple(MOVE_SPEED, 0.5); // go forward a bit
            }
            else if ((sensors & 0x01) == 0x01) // if sensor 0 is black
            {
                Motor_LeftSimple(MOVE_SPEED, 5); // turn right a significant amount
                Motor_ForwardSimple(MOVE_SPEED, 0.5); // go forward a bit
            }
            else if ((sensors & 0x40) == 0x40) // if sensor 6 is black
            {
                Motor_RightSimple(MOVE_SPEED, 5); // turn left a medium amount
                Motor_ForwardSimple(MOVE_SPEED, 1); // go forward a bit
            }
            else if ((sensors & 0x02) == 0x02) // if sensor 1 is black
            {
                Motor_LeftSimple(MOVE_SPEED, 5); // turn right a medium amount
                Motor_ForwardSimple(MOVE_SPEED, 0.5); // go forward a bit
            }
            else if ((sensors & 0x04) == 0x04) // if sensor 2 is black
            {
                Motor_LeftSimple(MOVE_SPEED, 5); // turn right a small amount
                Motor_ForwardSimple(MOVE_SPEED, 0.5); // go forward a bit
            }
            else if ((sensors & 0x20) == 0x20) // if sensor 5 is black
            {
                Motor_RightSimple(MOVE_SPEED, 5); // turn left a small amount
                Motor_ForwardSimple(MOVE_SPEED, 0.5); // go forward a bit
            }
            else if (((sensors & 0x10) == 0x10) || // if sensor 4 is black
                     ((sensors & 0x08) == 0x08)) // or if sensor 3 is black
            {
                Motor_ForwardSimple(MOVE_SPEED, 2);
            }
            lineSensors = LineSensor_Read();
        }
    }
}
