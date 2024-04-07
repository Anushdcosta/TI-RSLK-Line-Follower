#include "msp.h"

uint8_t lineSensors; // value last read from the line sensors

// State variables
enum State
{
    STOPPED, // was just turned on or was stopped part-way through running
    RUNNING, // solving the maze
    WIN, // found the treasure
    SOLUTIONING // showing the solution
};
enum State state; // the robot's state

#define MOVE_SPEED 4000// the standard movement speed of the robot while maze solving

