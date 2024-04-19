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
  EnumType status_code_;
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

  GeneralStatus getCopy() const
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
    for (const auto& status : underlying_status_messages_)
    {
      copy.underlying_status_messages_.push_back(
          std::make_unique<GeneralStatus>(status->getCopy()));
    }
    copy.uuid_ = uuid_;
    return copy;
  }
};

}   // namespace general_status
}   // namespace BT
#endif   // GENERAL_STATUS_INTERFACE_HPP
