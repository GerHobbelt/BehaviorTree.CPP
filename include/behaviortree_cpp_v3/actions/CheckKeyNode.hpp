#ifndef GET_CHECK_KEY_NODE_HPP
#define GET_CHECK_KEY_NODE_HPP

#include <iostream>
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class CheckKeyNode final : public BT::ActionNodeBase
{
    public:
        using BT::ActionNodeBase::ActionNodeBase;
        ~CheckKeyNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("yes_key", "Key to determine if key to say 'yes' has been pressed"),
                     BT::InputPort<std::string>("no_key", "Key to determine if key to say 'no' has benn pressed")
                   };
        }

    private:
        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);
            const auto& yes_key = getInput<std::string>("yes_key");
            const auto& no_key  = getInput<std::string>("no_key");

            if(!yes_key)   { throw BT::RuntimeError { name() + ": " + yes_key.error()   }; }
            if(!no_key)  { throw BT::RuntimeError { name() + ": " + no_key.error()  }; }

            std::string key="";
            auto status = BT::NodeStatus::RUNNING;

            while (key != yes_key && key != no_key) 
            {
                std::getline(std::cin, key);
            
                if (key == yes_key)  status = BT::NodeStatus::SUCCESS;
                else if (key == no_key) status = BT::NodeStatus::FAILURE;
            }  

            return status;
        }

        virtual void halt() override { setStatus(BT::NodeStatus::IDLE); }
};

}
#endif
