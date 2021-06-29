#ifndef CONCATENATE_STRINGS_NODE_HPP
#define CONCATENATE_STRINGS_NODE_HPP

#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
template <size_t NUM_STRINGS>
class ConcatenateStringsNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~ConcatenateStringsNode() = default;

        static BT::PortsList providedPorts()
        {
            BT::PortsList ports;
            ports.insert( BT::InputPort<std::string>("first", "First string") );
            ports.insert( BT::InputPort<std::string>("second", "Second string") );

            std::string port_name;
            for (unsigned i=3; i <= NUM_STRINGS; i++)
            {
                port_name = "string_" + std::to_string(i);
                ports.insert( BT::InputPort<std::string>(port_name) );
            }

            ports.insert( BT::OutputPort<std::string>("output", "Concatenated result string") );
            return ports;
        }

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);
            const auto& first  = getInput<std::string>("first");
            const auto& second = getInput<std::string>("second");

            if(!first)  { throw BT::RuntimeError { name() + ": " + first.error()  }; }
            if(!second) { throw BT::RuntimeError { name() + ": " + second.error() }; }

            std::string output_string = first.value() + second.value();

            std::string port_name;
            std::string value;
            bool found = false;
            for (unsigned i=3; i <= NUM_STRINGS; i++)
            {
                port_name = "string_" + std::to_string(i);
                found = (bool)getInput(port_name, value);
                if(!found) { throw BT::RuntimeError { name() + ": " + second.error() }; }

                output_string += value;
            }

            setOutput("output", output_string);
            return BT::NodeStatus::SUCCESS;
        }
};
}

#endif
