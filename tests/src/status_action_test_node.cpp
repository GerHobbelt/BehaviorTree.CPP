#include "status_action_test_node.h"

StatusActionTestNode::StatusActionTestNode(const std::string& name) :
  BT::ActionNodeBase(name, {})
{
  tick_count_ = 0;
  expected_result_ = BT::NodeStatus::IDLE;
  expected_code_ = BT::general_status::BtErrorCodes::OK;
  setGeneralStatusUpdateFunction(
      [&](TreeNode& tree_node, BT::NodeStatus node_status,
          BT::Optional<BT::general_status::GeneralStatus>& status) {
        TreeNode::defaultGeneralStatusUpdateCallback(*this, node_status, status);
        status->status_code_ = expected_code_; // override the default status code
      });
}

int StatusActionTestNode::tickCount() const
{
  return tick_count_;
}

void StatusActionTestNode::resetTicks()
{
  tick_count_ = 0;
}

void StatusActionTestNode::halt()
{
  resetTicks();
}

BT::NodeStatus StatusActionTestNode::tick()
{
  if (status() == BT::NodeStatus::IDLE)
    resetTicks();
  tick_count_++;
  if (tick_count_ == 1)
    return BT::NodeStatus::RUNNING;
  return expected_result_;
}

void StatusActionTestNode::setExpectedResult(BT::NodeStatus res)
{
  expected_result_ = res;
}

void StatusActionTestNode::setExpectedCode(BT::general_status::EnumType code)
{
  expected_code_ = code;
}

BT::general_status::EnumType StatusActionTestNode::getExpectedCode() const
{
  return expected_code_;
}