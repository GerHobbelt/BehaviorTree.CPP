#ifndef LOOP_NODE_HPP
#define LOOP_NODE_HPP

#include "behaviortree_cpp_v3/decorator_node.h"

namespace BT
{
class LoopNode final : public BT::DecoratorNode
{
    public:
        LoopNode(const std::string& _name) : BT::DecoratorNode(_name, {})
        {}
        ~LoopNode() = default;

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);
            child_node_->executeTick();

            return status();
        }
};
}

#endif
