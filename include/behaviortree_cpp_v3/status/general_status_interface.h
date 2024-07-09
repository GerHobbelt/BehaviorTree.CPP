//
// Created by porebski on 18.04.24.
//

#ifndef GENERAL_STATUS_INTERFACE_HPP
#define GENERAL_STATUS_INTERFACE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "behaviortree_cpp_v3/basic_types.h"

namespace BT
{
namespace general_status
{
using EnumType = int;

enum BtErrorCodes : EnumType
{
  OK = 0,
  BEHAVIOR_TREE_NODE_FAILURE = 2000000,
  BEHAVIOR_TREE_PARALLEL_NODE_MULTIPLE_FAILURES = 2000001,
};

struct Timestamp
{
  int64_t seconds_;
  int32_t nanos_;
};

struct ShuttleHardwareInfo
{
  uint64_t id_;
  EnumType specific_error_source_;
  Optional<uint32_t> additional_error_code_;
  Optional<std::string> error_description_;
  Optional<std::string> hardware_id_;
};

struct TaskInfo
{
  uint64_t id_;
  uint64_t task_id_;
  uint64_t host_id_;
  std::string host_name_;
};

struct GeneralStatus
{
  uint64_t id_;
  Timestamp timestamp_;
  EnumType status_code_ = BtErrorCodes::OK;
  Optional<EnumType> source_;
  Optional<ShuttleHardwareInfo> shuttle_hardware_info_;
  Optional<TaskInfo> task_info_;
  Optional<std::string> opt_string_;
  Optional<uint64_t> shuttle_id_;
  Optional<std::string> station_name_;
  std::vector<std::unique_ptr<GeneralStatus>> underlying_status_messages_;
  std::string uuid_;

  GeneralStatus() = default;
  GeneralStatus(const GeneralStatus&) = delete;
  GeneralStatus& operator=(const GeneralStatus&) = delete;
  GeneralStatus(GeneralStatus&&) = default;
  GeneralStatus& operator=(GeneralStatus&&) = default;

  GeneralStatus getShallowCopy() const
  {
    GeneralStatus copy;
    copy.id_ = id_;
    copy.timestamp_ = timestamp_;
    copy.status_code_ = status_code_;
    copy.source_ = source_;
    copy.shuttle_hardware_info_ = shuttle_hardware_info_;
    copy.task_info_ = task_info_;
    copy.opt_string_ = opt_string_;
    copy.shuttle_id_ = shuttle_id_;
    copy.station_name_ = station_name_;
    copy.uuid_ = uuid_;
    return copy;
  }

  GeneralStatus getCopy() const
  {
    auto copy = getShallowCopy();
    for (const auto& status : underlying_status_messages_)
    {
      copy.underlying_status_messages_.push_back(
          std::make_unique<GeneralStatus>(status->getCopy()));
    }
    return copy;
  }

  void mergeDataShallow(const GeneralStatus& other)
  {
    id_ = other.id_;
    timestamp_ = other.timestamp_;
    status_code_ = other.status_code_;
    source_ = other.source_;
    shuttle_hardware_info_ = other.shuttle_hardware_info_;
    task_info_ = other.task_info_;
    opt_string_ = other.opt_string_;
    shuttle_id_ = other.shuttle_id_;
    station_name_ = other.station_name_;
    uuid_ = other.uuid_;
  }
};

}   // namespace general_status
}   // namespace BT
#endif   // GENERAL_STATUS_INTERFACE_HPP
