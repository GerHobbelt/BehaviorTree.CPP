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
#include "behaviortree_cpp_v3/decorators/consume_queue.h"
#include "status_action_test_node.h"

using BT::NodeStatus;
using BT::general_status::BtErrorCodes;
using BT::general_status::EnumType;
using std::chrono::milliseconds;

class GeneralStatusDecoratorTest : public ::testing::Test
{
public:
  void SetUp() override
  {}

  void TearDown() override
  {}

  void initialize(NodeStatus actionResult, EnumType actionCode)
  {
    action_ = std::make_unique<StatusActionTestNode>("action");
    action_->setExpectedResult(actionResult);
    action_->setExpectedCode(actionCode);
    root_->setChild(action_.get());
  }

  void runTest(NodeStatus expectedResult, EnumType expectedCode,
               EnumType expectedChildCode, const std::string& desc)
  {
    // first tick - node should be running
    BT::NodeStatus state = root_->executeTick();
    if (root_->name() == "DelayNode")
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(defaultDelayMs_));
      state = root_->executeTick();
    }
    EXPECT_EQ(state, NodeStatus::RUNNING) << desc << ": Root node finished too soon";
    ASSERT_FALSE(root_->getGeneralStatus().has_value())
        << desc << ": Root node status already set";

    // second tick - check result
    state = root_->executeTick();
    EXPECT_EQ(state, expectedResult) << desc << ": Root node not finished";
    const auto& status = root_->getGeneralStatus();
    ASSERT_TRUE(status.has_value()) << desc << ": Root node status not set";
    EXPECT_EQ(status.value().status_code_, expectedCode)
        << desc << ": Root node status mismatch";

    EXPECT_EQ(action_->status(), NodeStatus::IDLE);
    const auto& child_status = action_->getGeneralStatus();
    ASSERT_TRUE(child_status.has_value()) << desc << ": Child node status not set";
    EXPECT_EQ(child_status.value().status_code_, expectedChildCode) << desc
                                                                    << ": Child node "
                                                                       "status "
                                                                       "mismatch";
  }

  void runFixture(NodeStatus childStatus, EnumType childCode, NodeStatus rootStatus,
                  EnumType rootCode, std::string desc)
  {
    createRoot_();
    initialize(childStatus, childCode);
    runTest(rootStatus, rootCode, childCode, desc);
  }

  void runDefaultFixtures()
  {
    // Assume decorator do nothing - just propagate error as is

    // child success with OK
    runFixture(NodeStatus::SUCCESS, BtErrorCodes::OK, NodeStatus::SUCCESS,
               BtErrorCodes::OK, "Ch: SUCCESS:OK, R: FAILURE:OK");

    // child success with warning
    runFixture(NodeStatus::SUCCESS, 13, NodeStatus::SUCCESS, BtErrorCodes::OK,
               "Ch: SUCCESS:13, R: SUCCESS:OK");

    // child failure with OK
    runFixture(NodeStatus::FAILURE, BtErrorCodes::OK, NodeStatus::FAILURE,
               BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE,
               "Ch: FAILURE:OK, R: FAILURE:BEHAVIOR_TREE_NODE_FAILURE");

    // child failure with error
    runFixture(NodeStatus::FAILURE, 154, NodeStatus::FAILURE, 154,
               "Ch: FAILURE:154, R: FAILURE:154");
  }

  void runRootFixture(NodeStatus rootStatus, EnumType rootCode, std::string desc = "")
  {
    createRoot_();
    initialize(NodeStatus::FAILURE, BtErrorCodes::OK);
    auto state = root_->executeTick();
    EXPECT_EQ(state, rootStatus) << "runRootFixture: Root node not finished";
    const auto& status = root_->getGeneralStatus();
    ASSERT_TRUE(status.has_value()) << "runRootFixture: Root node status not set";
    EXPECT_EQ(status.value().status_code_, rootCode) << "Root node status mismatch";

    EXPECT_EQ(action_->status(), NodeStatus::IDLE) << "runRootFixture: Child node not "
                                                      "idle";
    const auto& child_status = action_->getGeneralStatus();
    ASSERT_FALSE(child_status.has_value()) << "runRootFixture: Child node status set";
  }

protected:
  std::unique_ptr<BT::DecoratorNode> root_;
  std::unique_ptr<StatusActionTestNode> action_;

  std::function<void()> createRoot_;

  const int defaultDelayMs_ = 10;
};

/****************TESTS START HERE***************************/
TEST_F(GeneralStatusDecoratorTest, BlackboardPrecondition)
{
  auto bb = BT::Blackboard::create();
  BT::NodeConfiguration config;
  BT::assignDefaultRemapping<BT::BlackboardPreconditionNode<int>>(config);
  config.blackboard = bb;

  const auto answer = 42;
  config.blackboard->set<int>("value_A", answer);
  config.blackboard->set<int>("value_B", answer);
  config.blackboard->set<BT::NodeStatus>("return_on_mismatch", BT::NodeStatus::FAILURE);

  createRoot_ = [this, &config]() {
    root_ = std::make_unique<BT::BlackboardPreconditionNode<int>>("BlackboardPrecond",
                                                                  config);
  };

  runDefaultFixtures();

  // root failure -> report error and do not tick child
  config.blackboard->set<int>("value_A", 111);
  runRootFixture(NodeStatus::FAILURE, BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE);
}

TEST_F(GeneralStatusDecoratorTest, ConsumeQueue)
{
  // This node is rarely used so this test is not exhaustive

  auto bb = BT::Blackboard::create();
  BT::NodeConfiguration config;
  BT::assignDefaultRemapping<BT::ProtectedQueue<int>>(config);
  config.blackboard = bb;

  const auto answer = 42;
  config.blackboard->set<std::shared_ptr<BT::ProtectedQueue<int>>>(
      "queue", std::make_shared<BT::ProtectedQueue<int>>());
  createRoot_ = [this, &config]() {
    root_ = std::make_unique<BT::ConsumeQueue<int>>("ConsumeQueue", config);
  };

  // no queue -> success
  runRootFixture(NodeStatus::SUCCESS, BtErrorCodes::OK);
}

TEST_F(GeneralStatusDecoratorTest, DelayNode)
{
  // Name is an identifier for this test to work properly!
  createRoot_ = [&]() { root_ = std::make_unique<BT::DelayNode>("DelayNode", 1); };

  runDefaultFixtures();
}

TEST_F(GeneralStatusDecoratorTest, ForceFailureNode)
{
  createRoot_ = [&]() {
    root_ = std::make_unique<BT::ForceFailureNode>("ForceFailureNode");
  };

  // child success with OK
  runFixture(NodeStatus::SUCCESS, BtErrorCodes::OK, NodeStatus::FAILURE,
             BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE,
             "Ch: SUCCESS:OK, R: FAILURE:BEHAVIOR_TREE_NODE_FAILURE");

  // child success with warning
  runFixture(NodeStatus::SUCCESS, 13, NodeStatus::FAILURE, 13,
             "Ch: SUCCESS:13, R: FAILURE:13");

  // child failure with OK
  runFixture(NodeStatus::FAILURE, BtErrorCodes::OK, NodeStatus::FAILURE,
             BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE,
             "Ch: FAILURE:OK, R: FAILURE:BEHAVIOR_TREE_NODE_FAILURE");

  // child failure with error
  runFixture(NodeStatus::FAILURE, 154, NodeStatus::FAILURE, 154,
             "Ch: FAILURE:154, R: FAILURE:154");
}

TEST_F(GeneralStatusDecoratorTest, ForceSuccessNode)
{
  createRoot_ = [&]() {
    root_ = std::make_unique<BT::ForceSuccessNode>("ForceSuccessNode");
  };

  // child success with OK
  runFixture(NodeStatus::SUCCESS, BtErrorCodes::OK, NodeStatus::SUCCESS, BtErrorCodes::OK,
             "Ch: SUCCESS:OK, R: SUCCESS:OK");

  // child success with warning
  runFixture(NodeStatus::SUCCESS, 13, NodeStatus::SUCCESS, BtErrorCodes::OK,
             "Ch: SUCCESS:13, R: SUCCESS:OK");

  // child failure with OK
  runFixture(NodeStatus::FAILURE, BtErrorCodes::OK, NodeStatus::SUCCESS, BtErrorCodes::OK,
             "Ch: FAILURE:OK, R: SUCCESS:OK");

  // child failure with error
  runFixture(NodeStatus::FAILURE, 154, NodeStatus::SUCCESS, BtErrorCodes::OK,
             "Ch: FAILURE:154, R: SUCCESS:OK");
}

TEST_F(GeneralStatusDecoratorTest, Inverter)
{
  createRoot_ = [&]() { root_ = std::make_unique<BT::InverterNode>("inverter"); };

  // child success with OK
  runFixture(NodeStatus::SUCCESS, BtErrorCodes::OK, NodeStatus::FAILURE,
             BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE,
             "Ch: SUCCESS:OK, R: FAILURE:BEHAVIOR_TREE_NODE_FAILURE");

  // child success with warning
  runFixture(NodeStatus::SUCCESS, 13, NodeStatus::FAILURE, 13,
             "Ch: SUCCESS:13, R: FAILURE:13");

  // child failure with OK
  runFixture(NodeStatus::FAILURE, BtErrorCodes::OK, NodeStatus::SUCCESS, BtErrorCodes::OK,
             "Ch: FAILURE:OK, R: SUCCESS:OK");

  // child failure with error
  runFixture(NodeStatus::FAILURE, 154, NodeStatus::SUCCESS, BtErrorCodes::OK,
             "Ch: FAILURE:154, R: SUCCESS:OK");
}

TEST_F(GeneralStatusDecoratorTest, KeepRunningUntilFailureNode)
{
  createRoot_ = [&]() {
    root_ = std::make_unique<BT::KeepRunningUntilFailureNode>("KeepRunningUntilFailureNod"
                                                              "e");
  };

  // child failure with OK
  runFixture(NodeStatus::FAILURE, BtErrorCodes::OK, NodeStatus::FAILURE,
             BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE,
             "Ch: FAILURE:OK, R: FAILURE:BEHAVIOR_TREE_NODE_FAILURE");

  // child failure with error
  runFixture(NodeStatus::FAILURE, 154, NodeStatus::FAILURE, 154,
             "Ch: FAILURE:154, R: FAILURE:154");

  /* Repeat once and fail */
  EnumType firstCode = 123;
  EnumType secondCode = 321;
  createRoot_();
  initialize(NodeStatus::SUCCESS, firstCode);
  // first run - RUNNING
  BT::NodeStatus state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (1A)";
  // first run - SUCCESS
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (1B)";
  // second run - RUNNING
  action_->setExpectedResult(NodeStatus::FAILURE);
  action_->setExpectedCode(secondCode);
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (2A)";
  // third run - FAILURE
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::FAILURE) << "Root node not finished (2B)";

  // Check results - repeat node should contain executed children's status and the last children contains failure.
  const auto& status = root_->getGeneralStatus();
  ASSERT_TRUE(status.has_value()) << "Root node status not set";
  EXPECT_EQ(status.value().status_code_, secondCode) << "Root node status mismatch";

  ASSERT_EQ(status.value().underlying_status_messages_.size(), 1U) << "Root node do not "
                                                                      "store executed "
                                                                      "children status.";
  const auto& first_child_status = status.value().underlying_status_messages_.at(0);
  EXPECT_EQ(first_child_status->status_code_, firstCode) << "First child status mismatch";

  EXPECT_EQ(action_->status(), NodeStatus::IDLE);
  const auto& second_child_status = action_->getGeneralStatus();
  ASSERT_TRUE(second_child_status.has_value()) << "Child node status not set";
  EXPECT_EQ(second_child_status.value().status_code_, secondCode) << "Child node "
                                                                     "status "
                                                                     "mismatch";
}

TEST_F(GeneralStatusDecoratorTest, RepeatNode)
{
  createRoot_ = [&]() { root_ = std::make_unique<BT::RepeatNode>("RepeatNode", 1); };
  runDefaultFixtures();

  /* Repeat once and fail */
  EnumType firstCode = 123;
  EnumType secondCode = 321;
  root_ = std::make_unique<BT::RepeatNode>("RepeatNode", 3);
  initialize(NodeStatus::SUCCESS, firstCode);
  // first run - RUNNING
  BT::NodeStatus state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (1)";
  // second run - SUCCESS & RUNNING
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (2)";
  // third run - FAILURE
  action_->setExpectedResult(NodeStatus::FAILURE);
  action_->setExpectedCode(secondCode);
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::FAILURE) << "Root node not finished (3)";

  // Check results - repeat node should contain executed children's status and the last children contains failure.
  const auto& status = root_->getGeneralStatus();
  ASSERT_TRUE(status.has_value()) << "Root node status not set";
  EXPECT_EQ(status.value().status_code_, secondCode) << "Root node status mismatch";

  ASSERT_EQ(status.value().underlying_status_messages_.size(), 1U) << "Root node do not "
                                                                      "store executed "
                                                                      "children status.";
  const auto& first_child_status = status.value().underlying_status_messages_.at(0);
  EXPECT_EQ(first_child_status->status_code_, firstCode) << "First child status mismatch";

  EXPECT_EQ(action_->status(), NodeStatus::IDLE);
  const auto& second_child_status = action_->getGeneralStatus();
  ASSERT_TRUE(second_child_status.has_value()) << "Child node status not set";
  EXPECT_EQ(second_child_status.value().status_code_, secondCode) << "Child node "
                                                                     "status "
                                                                     "mismatch";
}

TEST_F(GeneralStatusDecoratorTest, RetryNode)
{
  createRoot_ = [&]() { root_ = std::make_unique<BT::RetryNode>("RetryNode", 1); };
  runDefaultFixtures();

  /* Fail once and retry */
  EnumType firstCode = 123;
  EnumType secondCode = 321;
  root_ = std::make_unique<BT::RetryNode>("RetryNode", 3);
  initialize(NodeStatus::FAILURE, firstCode);
  // first run - RUNNING
  BT::NodeStatus state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (1)";
  // second run - FAILURE & RUNNING
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node not running (2)";
  // third run - SUCCESS
  action_->setExpectedResult(NodeStatus::SUCCESS);
  action_->setExpectedCode(secondCode);
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::SUCCESS) << "Root node not finished (3)";

  // Check results - RetryNode should contain executed children's status and the last children contains second code (not propagated).
  const auto& status = root_->getGeneralStatus();
  ASSERT_TRUE(status.has_value()) << "Root node status not set";
  EXPECT_EQ(status.value().status_code_, BtErrorCodes::OK) << "Root node status mismatch";

  ASSERT_EQ(status.value().underlying_status_messages_.size(), 1U) << "Root node do not "
                                                                      "store executed "
                                                                      "children status.";
  const auto& first_child_status = status.value().underlying_status_messages_.at(0);
  EXPECT_EQ(first_child_status->status_code_, firstCode) << "First child status mismatch";

  EXPECT_EQ(action_->status(), NodeStatus::IDLE);
  const auto& second_child_status = action_->getGeneralStatus();
  ASSERT_TRUE(second_child_status.has_value()) << "Child node status not set";
  EXPECT_EQ(second_child_status.value().status_code_, secondCode) << "Child node "
                                                                     "status "
                                                                     "mismatch";
}

TEST_F(GeneralStatusDecoratorTest, SubtreeNode)
{
  createRoot_ = [&]() { root_ = std::make_unique<BT::SubtreeNode>("SubtreeNode"); };
  runDefaultFixtures();
}

TEST_F(GeneralStatusDecoratorTest, TimeoutNode)
{
  createRoot_ = [&]() { root_ = std::make_unique<BT::TimeoutNode<>>("TimeoutNode", 5); };

  runDefaultFixtures();

  // timeout
  createRoot_();
  initialize(NodeStatus::SUCCESS, 112);
  BT::NodeStatus state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::RUNNING) << "Root node finished too soon";
  ASSERT_FALSE(root_->getGeneralStatus().has_value()) << "Root node status already set";

  // second tick - timeout
  std::this_thread::sleep_for(std::chrono::milliseconds(defaultDelayMs_));
  state = root_->executeTick();
  EXPECT_EQ(state, NodeStatus::FAILURE) << "Root node not finished";
  const auto& status = root_->getGeneralStatus();
  ASSERT_TRUE(status.has_value()) << "Root node status not set";
  EXPECT_EQ(status.value().status_code_, BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE) << "Ro"
                                                                                      "ot"
                                                                                      " n"
                                                                                      "od"
                                                                                      "e "
                                                                                      "st"
                                                                                      "at"
                                                                                      "us"
                                                                                      " m"
                                                                                      "is"
                                                                                      "ma"
                                                                                      "tc"
                                                                                      "h";

  EXPECT_EQ(action_->status(), NodeStatus::IDLE);
  const auto& child_status = action_->getGeneralStatus();
  ASSERT_FALSE(child_status.has_value()) << "Child node status is set";
}