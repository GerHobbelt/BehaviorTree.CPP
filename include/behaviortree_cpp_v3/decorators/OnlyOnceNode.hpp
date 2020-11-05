#ifndef ONLY_ONCE_NODE_HPP
#define ONLY_ONCE_NODE_HPP

#include "behaviortree_cpp_v3/decorator_node.h"

namespace BT
{
class OnlyOnceNode final : public BT::DecoratorNode
{
    public:
        OnlyOnceNode(const std::string& _name) : BT::DecoratorNode(_name, {})
        {}
        ~OnlyOnceNode() = default;

        virtual BT::NodeStatus tick() override
        {
            setStatus(returned_child_status_);

            if(!executed_)
            {
                returned_child_status_ = child_node_->executeTick();
                setStatus(returned_child_status_);
                if(status() != BT::NodeStatus::RUNNING) { executed_ = true; }
            }

            return status();
        }

    private:
        bool executed_ { false };
        BT::NodeStatus returned_child_status_ { BT::NodeStatus::RUNNING };
};
}

#endif
