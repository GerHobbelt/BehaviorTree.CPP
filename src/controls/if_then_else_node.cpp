/* Copyright (C) 2020 Davide Faconti -  All Rights Reserved
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

#include "behaviortree_cpp_v3/controls/if_then_else_node.h"
#include "Arduino.h"


namespace BT
{

IfThenElseNode::IfThenElseNode(const std::string &name)
  : ControlNode::ControlNode(name, {} )
  , child_idx_(0)
{
  setRegistrationID("IfThenElse");
}

void IfThenElseNode::halt()
{
  child_idx_ = 0;
  ControlNode::halt();
    log_d("xxxxxx111");
}

NodeStatus IfThenElseNode::tick()
{
    log_d("xxxxxx1");
  const size_t children_count = children_nodes_.size();

  if(children_count != 2 && children_count != 3)
  {
    throw std::logic_error("IfThenElseNode must have either 2 or 3 children");
  }

  setStatus(NodeStatus::RUNNING);
    log_d("xxxxxx2");

  if (child_idx_ == 0)
  {
    log_d("xxxxxx3");
    NodeStatus condition_status = children_nodes_[0]->executeTick();

    if (condition_status == NodeStatus::RUNNING)
    {
    log_d("xxxxxx31");
      return condition_status;
    }
    else if (condition_status == NodeStatus::SUCCESS)
    {
      child_idx_ = 1;
    }
    else if (condition_status == NodeStatus::FAILURE)
    {
      if( children_count == 3){
        child_idx_ = 2;
      }
      else{
    log_d("xxxxxx32");
        return condition_status;
      }
    }
  }
    log_d("xxxxxx4 | %d",child_idx_);
  // not an else
  if (child_idx_ > 0)
  {
    NodeStatus status = children_nodes_[child_idx_]->executeTick();
    if (status == NodeStatus::RUNNING)
    {
    log_d("xxxxxx5");
      return NodeStatus::RUNNING;
    }
    else{
    log_d("xxxxxx6 %d",status);
      haltChildren();
      child_idx_ = 0;
      return status;
    }
  }

  throw std::logic_error("Something unexpected happened in IfThenElseNode");
}

}  // namespace BT
