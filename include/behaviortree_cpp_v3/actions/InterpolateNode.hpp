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
            return { BT::InputPort<Range>("input_range",  "Input value range. E.g. 1;6"),
                     BT::InputPort<Range>("output_range", "Output value range. E.g. -1;3"),
                     BT::InputPort<T>("input_value",      "Value to interpolate."),
                     BT::OutputPort<T>("output_value",    "Interpolated value."),
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            const auto& input_range_val  = getInput<Range>("input_range");
            const auto& output_range_val = getInput<Range>("output_range");
            const auto& input_val        = getInput<T>("input_value");

            if(!output_range_val) { throw BT::RuntimeError { name() + ": " + output_range_val.error()  }; }
            if(!input_range_val) { throw BT::RuntimeError { name() + ": " + input_range_val.error()  }; }
            if(!input_val)       { throw BT::RuntimeError { name() + ": " + input_val.error()  }; }

            Range input_range  = input_range_val.value();
            Range output_range = output_range_val.value();

            // Check ranges are actualy ranges
            if(input_range.front() == input_range.back() || output_range.front() == output_range.back()) { return BT::NodeStatus::FAILURE; }

            // Don't asume the range is going to be min, max
            T in_range_max  = (input_range.front() > input_range.back()) ? input_range.front() : input_range.back();
            T in_range_min  = (input_range.front() < input_range.back()) ? input_range.front() : input_range.back();
            T out_range_max = (output_range.front() > output_range.back()) ? output_range.front() : output_range.back();
            T out_range_min = (output_range.front() < output_range.back()) ? output_range.front() : output_range.back();

            // Check input value is in input range
            if(input_val.value() < in_range_min || input_val.value() > in_range_max) { return BT::NodeStatus::FAILURE; }

            try
            {
                const double delta = (input_val.value() - in_range_min) / std::fabs(1.0 * (in_range_max - in_range_min));
                const auto interpolated_value = out_range_min + std::fabs(out_range_max - out_range_min) * delta;

                setOutput("output_value", interpolated_value);
            }
            catch(const std::runtime_error& ex) { return BT::NodeStatus::FAILURE; }

            return BT::NodeStatus::SUCCESS;
        }
};

} // namespace BT_EXTENSION

#endif
