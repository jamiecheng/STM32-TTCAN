//
// Created by Jamie on 1-5-2021.
//

#ifdef NODE_A
#include "node_a.h"
#elif NODE_B
#include "node_b.h"
#endif

#include "stm32f0xx.h"

static void config_led()
{
    // 1. Enable the peripheral clock of GPIOA
    // 2. Select output mode (01) on PA5
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER5_0;
}

int main()
{
    SysTick_Config(48000);

    config_led();

#ifdef NODE_A
    NodeA node;
    node.run();
#elif NODE_B
    NodeB node;
    node.run();
#endif
}
