//
// Created by Jamie on 4-5-2021.
//

#ifndef TTCAN_NODE_BASE_H
#define TTCAN_NODE_BASE_H

#include "cni.h"

class NodeBase {
public:
    NodeBase();

    virtual void run() = 0;

protected:
    cni::CommNetworkInterface *m_comm;
};

#endif //TTCAN_NODE_BASE_H
