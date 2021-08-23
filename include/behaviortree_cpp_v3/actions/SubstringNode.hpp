#ifndef SUBSTRING_NODE_HPP
#define SUBSTRING_NODE_HPP

#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class SubstringNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~SubstringNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("input", "String to look into"),
                     BT::InputPort<std::vector<std::string>>("substrings", "Substrings to look for"),
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            const auto& input      = getInput<std::string>("input");
            const auto& substrings = getInput<std::vector<std::string>>("substrings");

            if(!input)      { throw BT::RuntimeError { name() + ": " + input.error()  }; }
            if(!substrings) { throw BT::RuntimeError { name() + ": " + substrings.error() }; }

            for(const auto& substring : substrings.value())
            {
                if(input.value().find(substring) != std::string::npos) { return BT::NodeStatus::SUCCESS; }
            }

            return BT::NodeStatus::FAILURE;
        }
};
}

#endif
