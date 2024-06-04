#ifndef STATUS_ACTION_TEST_NODE_H
#define STATUS_ACTION_TEST_NODE_H

#include "behaviortree_cpp_v3/action_node.h"

class StatusActionTestNode : public BT::ActionNodeBase
{
public:
  StatusActionTestNode(const std::string& name);
  BT::NodeStatus tick() override;
  void setExpectedResult(BT::NodeStatus res);
  void setExpectedCode(BT::general_status::EnumType code);
  BT::general_status::EnumType getExpectedCode() const;
  int tickCount() const;
  void resetTicks();
  void halt() override;

private:
  BT::NodeStatus expected_result_;
  BT::general_status::EnumType expected_code_;
  int tick_count_;
};

// Declare your class or functions here

#endif   // STATUS_ACTION_TEST_NODE_H