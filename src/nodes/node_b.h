//
// Created by Jamie on 4-5-2021.
//

#ifndef TTCAN_NODE_B_H
#define TTCAN_NODE_B_H

#ifdef NODE_B

#include "node_base.h"

struct SensorValues {
    uint64_t gas;
    uint64_t brake;
};

class NodeB : public NodeBase
{
public:
    NodeB();

    void run() override;

private:
    static void _cni_message_received(cni::msg_id_t id, uint32_t hr, uint32_t lr);

    cni::ScheduleMatrix m_matrix;
    static SensorValues m_sensors;
};
#endif

#endif //TTCAN_NODE_B_H