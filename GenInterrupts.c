
#include "msp.h"
#include "GenInterrupts.h"

// Disable interrupts.
void DisableInterrupts(void)
{
    __asm ("    CPSID  I\n"
            "    BX     LR\n");
}

// Enable interrupts.
void EnableInterrupts(void)
{
    __asm ("    CPSIE  I\n"
            "    BX     LR\n");
}

// Puts the MCU to sleep until an interrupt arrives.
void WaitForInterrupt(void)
{
  __asm  ("    WFI\n"
          "    BX     LR\n");
}
