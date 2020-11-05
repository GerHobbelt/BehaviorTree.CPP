#ifndef INTERPOLATE_NODE_HPP
#define INTERPOLATE_NODE_HPP

#include <type_traits>
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{

/*
 *
 * Interpolates a given input value in input_range to an output_range.
 * Output_range input port is optional. If left empty, the original input_range
 * will be used.
 *
 */
template<typename T>
class InterpolateNode final : public BT::SyncActionNode
{
    static_assert(std::is_arithmetic<T>::value,
            "InterpolateNode template argument must be an arithmetic value");

    private:
        using Range = std::vector<T>;

    public:
        using BT::SyncActionNode::SyncActionNode;
        ~InterpolateNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<Range>("input_range",  "Input value range."),
                     BT::InputPort<Range>("output_range", "Output value range."),
                     BT::InputPort<T>("input_value",      "Value to interpolate."),
                     BT::OutputPort<T>("output_value",    "Interpolated value."),
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            // TODO: add proper error handling
            setStatus(BT::NodeStatus::RUNNING);

            const auto& input_range_val  = getInput<Range>("input_range");
            const auto& output_range_val = getInput<Range>("output_range");
            const auto& input_val        = getInput<T>("input_value");

            Range input_range;
            Range output_range;

            if(!input_range_val) { throw BT::RuntimeError { name() + ": " + input_range_val.error()  }; }
            if(!input_val)       { throw BT::RuntimeError { name() + ": " + input_val.error()  }; }

            input_range = input_range_val.value();

            if(!output_range_val)
            {
                output_range = input_range;
            } 
            else
            {
                output_range = output_range_val.value();
            }

            const double delta = (input_val.value() - input_range.front()) / std::fabs(1.0 * (input_range.back() - input_range.front()));
            const auto interpolated_value = output_range.front() + std::fabs(output_range.back() - output_range.front()) * delta;

            setOutput("output_value", interpolated_value);

            return BT::NodeStatus::SUCCESS;
        }
};

} // namespace BT_EXTENSION

#endif
