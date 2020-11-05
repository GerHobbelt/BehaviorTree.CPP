#ifndef CONCATENATE_STRINGS_NODE_HPP
#define CONCATENATE_STRINGS_NODE_HPP

#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class ConcatenateStringsNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~ConcatenateStringsNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("first", "First string"),
                     BT::InputPort<std::string>("second", "Second string"),
                     BT::OutputPort<std::string>("output", "Concatenated result string")
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);
            const auto& first  = getInput<std::string>("first");
            const auto& second = getInput<std::string>("second");

            if(!first)  { throw BT::RuntimeError { name() + ": " + first.error()  }; }
            if(!second) { throw BT::RuntimeError { name() + ": " + second.error() }; }

            setOutput("output", first.value() + second.value());
            return BT::NodeStatus::SUCCESS;
        }
};
}

#endif
