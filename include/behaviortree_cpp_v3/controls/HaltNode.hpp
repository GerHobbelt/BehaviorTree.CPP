#ifndef HALT_NODE_HPP
#define HALT_NODE_HPP

#include "behaviortree_cpp_v3/control_node.h"

namespace BT
{
class HaltNode final : public BT::ControlNode
{
    public:
        using BT::ControlNode::ControlNode;
        ~HaltNode() = default;

        static BT::PortsList providedPorts()
        {
            return {};
        }

        virtual BT::NodeStatus tick() override
        {
            child_status_ = children_nodes_[0]->executeTick();
            return child_status_;
        }

        virtual void halt() override
        {
            if(child_status_ == BT::NodeStatus::RUNNING)
            {
                for(size_t i = 1; i < childrenCount(); i++)
                {
                    const BT::NodeStatus& halt_branch_status = children_nodes_[i]->executeTick();

                    if(halt_branch_status == BT::NodeStatus::RUNNING)
                    {
                        throw BT::RuntimeError { name() + ": halt branches shall never return RUNNING" };
                    }
                }
            }

            child_status_ = BT::NodeStatus::IDLE;
            ControlNode::halt();
        }

    private:
        BT::NodeStatus child_status_ { BT::NodeStatus::IDLE };
};
}

#endif
