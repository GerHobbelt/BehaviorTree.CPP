#ifndef RANDOMIZE_SEQUENCE_NODE_HPP
#define RANDOMIZE_SEQUENCE_NODE_HPP

#include "behaviortree_cpp/action_node.h"

#include "behaviortree_cpp/utils/random.hpp"

namespace BT
{
class RandomizeSequenceNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~RandomizeSequenceNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("sequence", "Sequence to randomize"),
                     BT::InputPort<std::string>("delimiter", ";", "Sequence entries delimiter"),
                     BT::OutputPort<std::string>("output", "Result random sequence entry")
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);
            const auto& raw_sequence = getInput<std::string>("sequence");
            const auto& delimiter    = getInput<std::string>("delimiter");

            if(!raw_sequence) { throw BT::RuntimeError { name() + ": " + raw_sequence.error() }; }
            if(!delimiter)    { throw BT::RuntimeError { name() + ": " + delimiter.error() }; }

            const auto& sequence = BT::splitString(raw_sequence.value(), delimiter.value().front());
            if(sequence.empty()) { return BT::NodeStatus::FAILURE; }

            setOutput("output", Utils::getRandomIterator(sequence.begin(), sequence.end())->to_string());

            return BT::NodeStatus::SUCCESS;
        }
};
}

#endif
