#ifndef BEHAVIORTREECORE_BLACKBOARD_UTIL_H
#define BEHAVIORTREECORE_BLACKBOARD_UTIL_H

#include "behaviortree_cpp/basic_types.h"

namespace BT
{
    bool       isBlackboardPointer(StringView str);
    StringView stripBlackboardPointer(StringView str);

}   // namespace BT

#endif
