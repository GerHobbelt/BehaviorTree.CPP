#ifndef RANDOMIZE_VALUE_NODE_HPP
#define RANDOMIZE_VALUE_NODE_HPP

#include "behaviortree_cpp_v3/action_node.h"

#include "behaviortree_cpp_v3/utils/random.hpp"

namespace BT
{
template <class T>
class RandomizeValueNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~RandomizeValueNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<T>("min", "Minimum range value"),
                     BT::InputPort<T>("max", "Max range value"),
                     BT::OutputPort<T>("result", "Random result value")
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            const auto& min = getInput<T>("min");
            const auto& max = getInput<T>("max");

            if(!min) { throw BT::RuntimeError { name() + ": " + min.error() }; }
            if(!max) { throw BT::RuntimeError { name() + ": " + max.error() }; }

            if(min.value() > max.value()) { throw BT::RuntimeError { name() + " : max value shall be greater than min." }; }

            setOutput("result", Utils::getRandomNumber<T>(min.value(), max.value()));
            return BT::NodeStatus::SUCCESS;
        }
};
}

#endif
