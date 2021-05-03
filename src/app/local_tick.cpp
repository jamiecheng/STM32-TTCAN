//
// Created by Jamie on 1-5-2021.
//

#include "local_tick.h"

static uint32_t local_tick;

void local_tick_increment()
{
    local_tick++;
}

uint32_t local_tick_get()
{
    return local_tick;
}
