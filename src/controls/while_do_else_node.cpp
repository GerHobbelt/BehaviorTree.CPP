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

#include "behaviortree_cpp_v3/controls/while_do_else_node.h"

namespace BT
{
WhileDoElseNode::WhileDoElseNode(const std::string& name) :
  ControlNode::ControlNode(name, {})
{
  setRegistrationID("WhileDoElse");
}

void WhileDoElseNode::halt()
{
  ControlNode::halt();
}

NodeStatus WhileDoElseNode::tick()
{
  const size_t children_count = children_nodes_.size();

  if (children_count != 3)
  {
    throw std::logic_error("WhileDoElse must have 3 children");
  }

  setStatus(NodeStatus::RUNNING);

  NodeStatus condition_status = children_nodes_[0]->executeTick();

  if (condition_status == NodeStatus::RUNNING)
  {
    return condition_status;
  }

  NodeStatus status = NodeStatus::IDLE;
  int child_index_to_execute = (condition_status == NodeStatus::SUCCESS) ? 1 : 2;
  int child_index_to_halt = (condition_status == NodeStatus::SUCCESS) ? 2 : 1;
  haltChild(child_index_to_halt);
  status = children_nodes_[child_index_to_execute]->executeTick();
  if (status == NodeStatus::FAILURE)
  {
    propagateGeneralStatusFromFailingChild(children_nodes_[child_index_to_execute]);
  }
  if (status == NodeStatus::RUNNING)
  {
    return NodeStatus::RUNNING;
  }
  else
  {
    resetChildren();
    return status;
  }
}

}   // namespace BT
