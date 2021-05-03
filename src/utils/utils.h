//
// Created by Jamie on 3-5-2021.
//

#ifndef TTCAN_UTILS_H
#define TTCAN_UTILS_H

#include "local_tick.h"

namespace utils
{
void delay(uint32_t ms)
{
    uint32_t tick_end = local_tick_get() + ms;

    while(local_tick_get() < tick_end)
    {
        __WFI();
    }
}
}

#endif //TTCAN_UTILS_H
