#ifndef O_STREAM_NODE_HPP
#define O_STREAM_NODE_HPP

#include <iostream>
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class OStreamNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~OStreamNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("message", "String to print") };
        }

        virtual BT::NodeStatus tick() override
        {
            const auto& input = getInput<std::string>("message");
            if(!input) { throw BT::RuntimeError { name() + ": " + input.error() }; }

            std::cout << input.value() << std::endl;
            return BT::NodeStatus::SUCCESS;
        }
};
}

#endif
