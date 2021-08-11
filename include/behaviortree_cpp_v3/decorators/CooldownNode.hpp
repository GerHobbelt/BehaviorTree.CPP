#ifndef COOLDOWN_NODE_HPP
#define COOLDOWN_NODE_HPP

#include "behaviortree_cpp_v3/decorator_node.h"
#include "behaviortree_cpp_v3/decorators/timer_queue.h"

namespace BT
{
template <class T>
class CooldownNode final : public BT::DecoratorNode
{
    template <typename R>
    struct isDuration : std::false_type {};

    template <typename Rep, typename Period>
    struct isDuration<std::chrono::duration<Rep, Period>> : std::true_type {};

    static_assert(isDuration<T>::value, "CooldownNode template argument must be a std::chrono::duration");

    public:
        using BT::DecoratorNode::DecoratorNode;
        ~CooldownNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<unsigned>("cooldown", 0, "Cooldown time") };
        }

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);
            if(in_cooldown_) { return last_child_status_; }

            last_child_status_ = child_node_->executeTick();

            if(last_child_status_ != BT::NodeStatus::RUNNING)
            {
                const auto& cooldown_time_raw = getInput<unsigned>("cooldown");
                if(!cooldown_time_raw) { throw BT::RuntimeError { name() + ": " + cooldown_time_raw.error() }; }

                T cooldown_time { cooldown_time_raw.value() };

                in_cooldown_ = true;

                timer().add(std::chrono::duration_cast<std::chrono::milliseconds>(cooldown_time),
                        [this] (bool _aborted)
                        {
                            if(!_aborted)
                            {
                                haltChild();
                                in_cooldown_ = false;
                            }
                        });
            }

            return last_child_status_;
        }

        virtual void halt() override
        {
            timer().cancelAll();
            in_cooldown_ = false;
            DecoratorNode::halt();
        }

    private:
        static BT::TimerQueue<>& timer()
        {
            static BT::TimerQueue<> timer_queue;
            return timer_queue;
        }

    private:
        std::atomic<bool> in_cooldown_    { false };
        BT::NodeStatus last_child_status_ { BT::NodeStatus::IDLE };
};
}

#endif
