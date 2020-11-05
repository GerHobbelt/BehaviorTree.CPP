#ifndef WHILE_SUCCESS_NODE_HPP
#define WHILE_SUCCESS_NODE_HPP

#include "behaviortree_cpp_v3/decorator_node.h"

namespace BT
{
class WhileSuccessNode final : public BT::DecoratorNode
{
    public:
        WhileSuccessNode(const std::string& _name) : BT::DecoratorNode(_name, {})
        {}
        ~WhileSuccessNode() = default;

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);

            const auto child_status = child_node_->executeTick();

            if(child_status == BT::NodeStatus::FAILURE)
            {
                setStatus(BT::NodeStatus::FAILURE);
                child_node_->setStatus(BT::NodeStatus::IDLE);
            }

            return status();
        }
};
}

#endif
