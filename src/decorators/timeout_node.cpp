/*  Copyright (C) 2018 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "behaviortree_cpp/decorators/timeout_node.h"

namespace BT
{
TimeoutNode::TimeoutNode(const std::string& name, unsigned milliseconds)
  : DecoratorNode(name, {}), timeout_reached_(false), msec_(milliseconds),
    read_parameter_from_blackboard_(false)
{
    setRegistrationName("Timeout");
}

TimeoutNode::TimeoutNode(const std::string& name, const BT::NodeParameters& params)
  : DecoratorNode(name, params), timeout_reached_(false), msec_(0)
{
    read_parameter_from_blackboard_ = isBlackboardPattern( params.at("msec") );
    if(!read_parameter_from_blackboard_)
    {
        if( !getParam("msec", msec_) )
        {
            throw std::runtime_error("Missing parameter [msec] in TimeoutNode");
        }
    }
}

NodeStatus TimeoutNode::tick()
{
    if( read_parameter_from_blackboard_ )
    {
        if( !getParam("msec", msec_) )
        {
            throw std::runtime_error("Missing parameter [msec] in TimeoutNode");
        }
    }

    if (status() == NodeStatus::IDLE)
    {
        setStatus(NodeStatus::RUNNING);
        timeout_reached_ = false;

        if (msec_ > 0)
        {
            timer().add(std::chrono::milliseconds(msec_), [this](bool aborted) {
                if (!aborted && child()->status() == NodeStatus::RUNNING)
                {
                    timeout_reached_ = true;
                }
            });
        }
    }

    const auto child_status = child()->executeTick();
    if(timeout_reached_)
    {
        child()->halt();
        setStatus(NodeStatus::FAILURE);
        return status();
    }

    if (child_status != NodeStatus::RUNNING)
    {
        timer().cancelAll();
    }

    setStatus(child_status);
    return status();
}

void TimeoutNode::halt()
{
    timer().cancelAll();
    DecoratorNode::halt();
}

}
