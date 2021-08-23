#ifndef GET_KEYBOARD_NODE_HPP
#define GET_KEYBOARD_NODE_HPP

#include <iostream>
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class GetKeyboardNode final : public BT::ActionNodeBase
{
    public:
        using BT::ActionNodeBase::ActionNodeBase;
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

        virtual void halt() override { setStatus(BT::NodeStatus::IDLE); }
};

}
#endif
