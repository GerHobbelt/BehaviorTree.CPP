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
#include "behaviortree_cpp_v3/actions/always_failure_node.h"
#include "status_action_test_node.h"

using BT::NodeStatus;
using BT::general_status::BtErrorCodes;
using BT::general_status::EnumType;
using std::chrono::milliseconds;

class GeneralStatusControlTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    actions_.emplace_back(std::make_unique<StatusActionTestNode>("A"));
    actions_.emplace_back(std::make_unique<StatusActionTestNode>("B"));
    actions_.emplace_back(std::make_unique<StatusActionTestNode>("C"));
    actions_[0]->setExpectedCode(13);
    actions_[1]->setExpectedCode(123);
    actions_[2]->setExpectedCode(1234);
  }

  void TearDown() override
  {}

protected:
  std::unique_ptr<BT::ControlNode> root_;
  std::vector<std::unique_ptr<StatusActionTestNode>> actions_;

  std::function<void()> createRoot_;

  void setExpectedResults(std::vector<NodeStatus> results)
  {
    for (size_t i = 0; i < results.size(); i++)
    {
      actions_.at(i)->setExpectedResult(results[i]);
    }
  }

  EnumType getExpectedCode(int idx)
  {
    return actions_.at(idx)->getExpectedCode();
  }

  void runTest(std::vector<NodeStatus> expectedStatuses, EnumType expectedCode,
               std::string name)
  {
    resetRoot();
    for (size_t i = 0; i < expectedStatuses.size(); i++)
    {
      EXPECT_EQ(root_->executeTick(), expectedStatuses.at(i))
          << "Test " << name << " step " << i << " status mismatch";
      if (i < expectedStatuses.size() - 1)
        EXPECT_FALSE(root_->getGeneralStatus().has_value())
            << "Test " << name << " step " << i << " status present";
    }
    const auto& status = root_->getGeneralStatus();
    ASSERT_TRUE(status.has_value()) << "Test " << name << " final status not set";
    EXPECT_EQ(status.value().status_code_, expectedCode)
        << "Test " << name << " final code mismatch";
  }

  void resetRoot()
  {
    root_->halt();   // this should be already called, but it is still sometimes needed
    root_->resetGeneralStatus();
  }
};

/****************TESTS START HERE***************************/
TEST_F(GeneralStatusControlTest, Fallback)
{
  // Fallback
  root_ = std::make_unique<BT::FallbackNode>("FallbackNode");
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());

  // 1: A(FAIL) -> B(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "1");

  // 2: A(FAIL) -> B(FAIL) -> OUTPUT: FAIL (B)
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "2");

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, IfThenElse)
{
  // [BUG]: IfThenElseNode is not halting child A if there is no child C

  // IfThenElse (A&B only)
  root_ = std::make_unique<BT::IfThenElseNode>("IfThenElseNode");
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());

  // 1(no C): A(FAIL) -> FAIL (generic error)
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE},
          BtErrorCodes::BEHAVIOR_TREE_NODE_FAILURE, "1");

  // Add third child (C)
  root_->addChild(actions_[2].get());

  // 2: A(FAIL) -> C(FAIL) -> FAIL
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(2), "2");

  // 3: A(FAIL) -> C(OK) -> OK
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "3");

  // 4: A(OK) -> B(FAIL) -> FAIL
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "4");

  // 5: A(OK) -> B(OK) -> OK
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "5");

  SUCCEED();
}

// Manual node is not trivial to test automatically
// TEST_F(GeneralStatusControlTest, Manual)
// {
//   SUCCEED();
// }

TEST_F(GeneralStatusControlTest, Parallel)
{
  // Parallel
  // Priority based propagation in case of multiples failures
  root_ = std::make_unique<BT::ParallelNode>("ParallelNode", -1, 2);
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());
  root_->addChild(actions_[2].get());

  // 1: A(OK), B(FAIL), C(FAIL) -> OUTPUT: FAIL(B)
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "1");

  // 2: A(OK), B(OK), C(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::SUCCESS}, BtErrorCodes::OK, "2");

  // 3: A(FAIL), B(FAIL), C(FAIL) -> OUTPUT: FAIL(A)
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(0), "3");  

  // 4: A(OK), B(FAIL), C(OK) -> OUTPUT: FAIL(B)
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "4");  

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, ReactiveFallback)
{
  // ReactiveFallback
  // Reactive fallback resets & retries all previous children when a child is running.
  // We need to use immediate nodes here, because otherwise it will go into infinite loop.
  root_ = std::make_unique<BT::ReactiveFallback>("ReactiveFallback");
  auto action0 = std::make_unique<BT::AlwaysFailureNode>("A");
  auto action1 = std::make_unique<BT::AlwaysFailureNode>("B");
  root_->addChild(action0.get());
  root_->addChild(action1.get());
  root_->addChild(actions_[0].get());

  // 1: A(FAIL) -> B(FAIL) -> C(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::SUCCESS});
  resetRoot();
  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "root status mismatch (1)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::SUCCESS) << "root status mismatch (2)";
  EXPECT_EQ(root_->getGeneralStatus().value().status_code_, BtErrorCodes::OK);
  // expect 2 children: A,B
  EXPECT_EQ(root_->getGeneralStatus().value().underlying_status_messages_.size(), 2U);
  // BT::printGeneralStatusRecursively(root_->getGeneralStatus().value());  // debug

  // 2: A(FAIL) -> B(FAIL) -> C(FAIL) -> OUTPUT: FAIL
  setExpectedResults({NodeStatus::FAILURE});
  resetRoot();
  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "root status mismatch (1)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::FAILURE) << "root status mismatch (2)";
  EXPECT_EQ(root_->getGeneralStatus().value().status_code_, getExpectedCode(0));
  // expect 2 children: A,B
  EXPECT_EQ(root_->getGeneralStatus().value().underlying_status_messages_.size(), 2U);
  // BT::printGeneralStatusRecursively(root_->getGeneralStatus().value());  // debug

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, ReactiveSequence)
{
  // ReactiveSequence
  // Reactive sequence resets & retries all previous children when a child is running.
  // We need to use immediate nodes here, because otherwise it will go into infinite loop.
  root_ = std::make_unique<BT::ReactiveSequence>("ReactiveFallback");
  auto action0 = std::make_unique<BT::AlwaysSuccessNode>("A");
  auto action1 = std::make_unique<BT::AlwaysSuccessNode>("B");
  root_->addChild(action0.get());
  root_->addChild(action1.get());
  root_->addChild(actions_[0].get());

  // 1: A(OK) -> B(OK) -> C(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::SUCCESS});
  resetRoot();
  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "root status mismatch (1)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::SUCCESS) << "root status mismatch (2)";
  EXPECT_EQ(root_->getGeneralStatus().value().status_code_, BtErrorCodes::OK);
  // expect 2 children: A,B
  EXPECT_EQ(root_->getGeneralStatus().value().underlying_status_messages_.size(), 2U);
  // BT::printGeneralStatusRecursively(root_->getGeneralStatus().value());   // debug

  // 2: A(OK) -> B(OK) -> C(FAIL) -> OUTPUT: FAIL
  setExpectedResults({NodeStatus::FAILURE});
  resetRoot();
  EXPECT_EQ(root_->executeTick(), NodeStatus::RUNNING) << "root status mismatch (1)";
  EXPECT_EQ(root_->executeTick(), NodeStatus::FAILURE) << "root status mismatch (2)";
  EXPECT_EQ(root_->getGeneralStatus().value().status_code_, getExpectedCode(0));
  // expect 2 children: A,B
  EXPECT_EQ(root_->getGeneralStatus().value().underlying_status_messages_.size(), 2U);
  // BT::printGeneralStatusRecursively(root_->getGeneralStatus().value());   // debug

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, Sequence)
{
  // Sequence
  root_ = std::make_unique<BT::SequenceNode>("SequenceNode");
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());

  // 1: A(OK) -> B(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "1");

  // 2: A(OK) -> B(FAIL) -> OUTPUT: FAIL (B)
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "2");

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, SequenceStar)
{
  // SequenceStarNode
  // The only difference between Sequence and SequenceStar it that the failed node is not reset
  // It does not affect propagation
  root_ = std::make_unique<BT::SequenceStarNode>("SequenceStarNode");
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());

  // 1: A(OK) -> B(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "1");

  // 2: A(OK) -> B(FAIL) -> OUTPUT: FAIL (B)
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "2");

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, Switch)
{
  // SwitchNode
  BT::NodeConfiguration config;
  BT::assignDefaultRemapping<BT::SwitchNode<2>>(config);
  config.blackboard = BT::Blackboard::create();

  root_ = std::make_unique<BT::SwitchNode<2>>("SwitchNode", config);
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());
  root_->addChild(actions_[2].get());

  // 1: no variable -> use default(C) C(FAIL) -> OUTPUT: FAIL
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE}, getExpectedCode(2), "1");

  config.blackboard->set<std::string>("variable", "F");
  config.blackboard->set<std::string>("case_1", "A");
  config.blackboard->set<std::string>("case_2", "B");

  // 2: variable do not match case -> use default(C) C(OK) -> OUTPUT: OK
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::SUCCESS}, BtErrorCodes::OK, "2");

  // 3: variable == A -> A(FAIL) -> OUTPUT: FAIL
  config.blackboard->set<std::string>("variable", "A");
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE}, getExpectedCode(0), "3");

  // 4: variable == B -> B(FAIL) -> OUTPUT: FAIL
  config.blackboard->set<std::string>("variable", "B");
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::FAILURE}, getExpectedCode(1), "4");

  SUCCEED();
}

TEST_F(GeneralStatusControlTest, WhileDoElse)
{
  // WhileDoElseNode
  root_ = std::make_unique<BT::WhileDoElseNode>("WhileDoElseNode");
  root_->addChild(actions_[0].get());
  root_->addChild(actions_[1].get());
  root_->addChild(actions_[2].get());

  // 2: A(FAIL) -> C(FAIL) -> FAIL
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE, NodeStatus::FAILURE});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(2), "2");

  // 3: A(FAIL) -> C(OK) -> OK
  setExpectedResults({NodeStatus::FAILURE, NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "3");

  // 4: A(OK) -> B(FAIL) -> FAIL
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::FAILURE, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::FAILURE},
          getExpectedCode(1), "4");

  // 5: A(OK) -> B(OK) -> OK
  setExpectedResults({NodeStatus::SUCCESS, NodeStatus::SUCCESS, NodeStatus::SUCCESS});
  runTest({NodeStatus::RUNNING, NodeStatus::RUNNING, NodeStatus::SUCCESS},
          BtErrorCodes::OK, "5");

  SUCCEED();
}