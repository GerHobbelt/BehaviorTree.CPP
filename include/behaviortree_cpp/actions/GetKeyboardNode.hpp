#ifndef GET_KEYBOARD_NODE_HPP
#define GET_KEYBOARD_NODE_HPP

#include <iostream>
#include "behaviortree_cpp/action_node.h"

namespace BT
{
class GetKeyboardNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~GetKeyboardNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::OutputPort<std::string>("output", "String received from keyboard")
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);

            std::string key;

            std::getline(std::cin, key);

            setOutput("output", key);
            return BT::NodeStatus::SUCCESS;
        }

};

}
#endif
