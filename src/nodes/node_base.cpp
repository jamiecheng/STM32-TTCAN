//
// Created by Jamie on 4-5-2021.
//

#include "node_base.h"
NodeBase::NodeBase()
{
    m_comm = cni::CommNetworkInterface::get_instance();
}
