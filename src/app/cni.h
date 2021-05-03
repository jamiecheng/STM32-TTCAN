//
// Created by Jamie on 1-5-2021.
//

#ifndef TTCAN_CNI_H
#define TTCAN_CNI_H

#include "schedule_matrix.h"
#include "stm32f0xx.h"

namespace cni
{
typedef enum
{
    MSG_ID_REF = 0x0110U,
    MSG_ID_ENGINE_TEMP = 0x0210U,
    MSG_ID_RPM = 0x0310U,
} msg_id_t;

typedef struct
{
    msg_id_t id;
    uint8_t basic_cycle;
    uint8_t column;
    uint64_t *data;
} schedule_tx_t;

class CommNetworkInterface
{
    using rx_cb = void (*)(msg_id_t, uint32_t, uint32_t);

public:
    CommNetworkInterface();

    static CommNetworkInterface *get_instance();

    void set_schedule_matrix(ScheduleMatrix *matrix);

    void set_master(bool master);

    void attach_rx_cb(rx_cb cb);

    void schedule_tx_trigger(schedule_tx_t *tx_trigger);

    void start();

    void can_isr();

    void tim_isr();

private:
    void _config_can();

    void _config_timer();

    uint8_t _transmit_message(uint32_t id, const uint64_t *data);

    static CommNetworkInterface m_inst;
    ScheduleMatrix *m_matrix{};
    rx_cb m_rx_cb{};
    schedule_tx_t *m_scheduled_tx[64]{};
    bool m_is_master{};
};
}

#endif //TTCAN_CNI_H
