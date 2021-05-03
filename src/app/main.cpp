//
// Created by Jamie on 1-5-2021.
//

#include "stm32f0xx.h"
#include "schedule_matrix.h"
#include "utils.h"
#include "cni.h"

static uint64_t engine_temp;
static uint64_t engine_rpm;

static void config_led()
{
    // 1. Enable the peripheral clock of GPIOA
    // 2. Select output mode (01) on PA5
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER5_0;
}

static void cni_rx_cb(cni::msg_id_t id, uint32_t hr, uint32_t lr)
{
    switch (id)
    {
        case cni::MSG_ID_ENGINE_TEMP:break;
        case cni::MSG_ID_REF:break;
        case cni::MSG_ID_RPM:break;
        default:break;
    }
}

int main()
{
    SysTick_Config(48000);

    config_led();

    // first, define our network transmission matrix
    cni::ScheduleMatrix matrix(1, 3);

    matrix.set_column_duration(0, 10);
    matrix.set_column_duration(1, 20);
    matrix.set_column_duration(2, 50);

    // register the matrix to the CAN interface
    auto comm = cni::CommNetworkInterface::get_instance();

    comm->set_schedule_matrix(&matrix);

    comm->set_master(true);

    // attach rx_trigger callback
    comm->attach_rx_cb(cni_rx_cb);

    // schedule tx_triggers
    cni::schedule_tx_t tx_engine_temp{
        .id = cni::MSG_ID_ENGINE_TEMP,
        .basic_cycle = 0,
        .column = 1,
        .data = &engine_temp
    };

    cni::schedule_tx_t tx_engine_rpm{
        .id = cni::MSG_ID_RPM,
        .basic_cycle = 0,
        .column = 2,
        .data = &engine_rpm
    };

    comm->schedule_tx_trigger(&tx_engine_temp);
    comm->schedule_tx_trigger(&tx_engine_rpm);

    comm->start();

    for (;;)
    {
        GPIOA->ODR ^= GPIO_ODR_5;
        utils::delay(100);
    }
}
