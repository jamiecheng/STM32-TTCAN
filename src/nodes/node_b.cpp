//
// Created by Jamie on 4-5-2021.
//

#ifdef NODE_B

#include <cstdlib>
#include "node_b.h"
#include "utils.h"

SensorValues NodeB::m_sensors;

NodeB::NodeB() : m_matrix(2, 3)
{
    m_matrix.set_column_duration(0, 1000);
    m_matrix.set_column_duration(1, 2000);
    m_matrix.set_column_duration(2, 5000);

    m_comm->set_schedule_matrix(&m_matrix);
    m_comm->set_master(false);
    m_comm->attach_rx_cb(NodeB::_cni_message_received);

    // schedule transmissions
    cni::schedule_tx_t tx_engine_temp{
        .id = cni::MSG_ID_GAS,
        .basic_cycle = 0,
        .column = 2,
        .data = &m_sensors.gas
    };

    cni::schedule_tx_t tx_engine_rpm{
        .id = cni::MSG_ID_BRAKE,
        .basic_cycle = 1,
        .column = 1,
        .data = &m_sensors.brake
    };

    m_comm->schedule_tx_trigger(&tx_engine_temp);
    m_comm->schedule_tx_trigger(&tx_engine_rpm);
}

void NodeB::run()
{
    m_comm->start();

    for(;;)
    {
        m_sensors.brake = rand();
        m_sensors.gas = rand();

        GPIOA->ODR ^= GPIO_ODR_5;

        utils::delay(100);
    }
}

void NodeB::_cni_message_received(cni::msg_id_t id, uint32_t hr, uint32_t lr)
{
    switch (id)
    {
        case cni::MSG_ID_ENGINE_TEMP:break;
        case cni::MSG_ID_RPM:break;
        default:break;
    }
}
#endif