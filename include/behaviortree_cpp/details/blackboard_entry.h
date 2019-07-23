#ifndef BLACKBOARD_ENTRY_H
#define BLACKBOARD_ENTRY_H

#include <set>
#include <type_index>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/utils/types_converter.hpp"
#include "behaviortree_cpp/exceptions.h"

namespace BT
{
namespace details
{
    class Entry
    {
        public:
            Entry(const PortInfo& _port_info, const TypesConverter& _converter);
            Entry(Any&& other_any, const PortInfo& _port_info, const TypesConverter& _converter);
            Entry(Any&& other_any, const PortDirection _direction, const TypesConverter& _converter);

            template<class T>
            T get();

            void addType(const PortInfo& _info);
            void addType(const std::type_info* _type, const PortDirection _direction);

        private:
            bool AreTypesCompatible();

        private:
            Any value_;

            using Types = std::set<std::type_index>;
            Types input_types_;
            Types output_types_;

            const TypesConverter& converter_;
    };
};

} // end namespace details
} // end namespace BT

#endif   // BLACKBOARD_ENTRY_H
