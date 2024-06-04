/* Copyright (C) 2015-2017 Michele Colledanchise - All Rights Reserved
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

#include <gtest/gtest.h>
#include "action_test_node.h"
#include "behaviortree_cpp_v3/behavior_tree.h"
#include "status_action_test_node.h"

using BT::NodeStatus;
using BT::general_status::BtErrorCodes;
using BT::general_status::EnumType;
using std::chrono::milliseconds;

class GeneralStatusControlTest : public ::testing::Test
{
public:
  void SetUp() override
  {}

  void TearDown() override
  {}

protected:
  std::unique_ptr<BT::ControlNode> root_;
  std::vector<std::unique_ptr<StatusActionTestNode>> actions_;

  std::function<void()> createRoot_;
};

/****************TESTS START HERE***************************/
TEST_F(GeneralStatusControlTest, Fallback)
{
  // Fallback
  // 1: A(FAIL) -> B(OK) -> OUTPUT: OK
  actions_.push_back(std::make_unique<StatusActionTestNode>("A"));
  actions_.push_back(std::make_unique<StatusActionTestNode>("B"));
  actions_[0]->setExpectedCode(13);
  actions_[1]->setExpectedCode(123);

  root_ = std::make_unique<BT::FallbackNode>("FallbackNode");
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());

  actions_[0]->setExpectedResult(NodeStatus::FAILURE);
  actions_[1]->setExpectedResult(NodeStatus::SUCCESS);

  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "First node is running";
  EXPECT_FALSE(root_->getGeneralStatus().has_value()) << "Root node status present (1)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "Second node is running";
  EXPECT_FALSE(root_->getGeneralStatus().has_value()) << "Root node status present (2)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::SUCCESS) << "Fallback return success";

  const auto& status = root_->getGeneralStatus();
  ASSERT_TRUE(status.has_value()) << "Root node status not set";
  EXPECT_EQ(status.value().status_code_, BtErrorCodes::OK) << "Root node "
                                                              "status "
                                                              "mismatch";
  root_->halt();
  root_->resetGeneralStatus();

  // 2: A(FAIL) -> B(FAIL) -> OUTPUT: FAIL (B)
  actions_[0]->setExpectedResult(NodeStatus::FAILURE);
  actions_[1]->setExpectedResult(NodeStatus::FAILURE);

  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "First node is running";
  EXPECT_FALSE(root_->getGeneralStatus().has_value()) << "Root node status present (1)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "Second node is running";
  EXPECT_FALSE(root_->getGeneralStatus().has_value()) << "Root node status present (2)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::FAILURE) << "Fallback return failure";

  const auto& status2 = root_->getGeneralStatus();
  ASSERT_TRUE(status2.has_value()) << "Root node status not set";
  EXPECT_EQ(status2.value().status_code_, actions_[1]->getExpectedCode()) << "Root node "
                                                                             "status "
                                                                             "mismatch";

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, IfThenElse)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, Manual)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, Parallel)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, ReactiveFallback)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, ReactiveSequence)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, Sequence)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, SequenceStar)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, Switch)
{
  SUCCEED();
}

TEST_F(GeneralStatusControlTest, WhileDoElse)
{
  SUCCEED();
}