#ifndef BLACKBOARD_ENTRY_H
#define BLACKBOARD_ENTRY_H

#include <set>
#include <typeindex>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/utils/types_converter.hpp"
#include "behaviortree_cpp/exceptions.h"

namespace BT
{
    class Entry
    {
        public:
            Entry(const Entry& _other_entry, const TypesConverter& _converter);
            Entry(const PortInfo& _port_info, const TypesConverter& _converter);
            Entry(Any&& other_any, const TypesConverter& _converter);
            Entry(Any&& other_any, const PortInfo& _port_info, const TypesConverter& _converter);
            Entry(Any&& other_any, const PortDirection _direction, const TypesConverter& _converter);

            template<class T>
            T getValue()
            {
                //TODO: checks on the setter and getter could be removed if more strict rules where applied
                //to the nodes. For example, if they could only request in the tick() functions the same type
                //they have specified in their port list (this would mean getting rid off the whole void stuff)
                addPortInfo(typeid(T), PortDirection::INPUT);
                return converter_.convert<T>(value_);
            }

            template<class T>
            void setValue(T&& _value)
            {
                addPortInfo(typeid(T), PortDirection::OUTPUT);
                value_ = Any(_value);
            }

            void addPortInfo(const PortInfo& _info);
            void addPortInfo(const std::type_info& _type, const PortDirection _direction);

        private:
            using Types = std::set<std::type_index>;
            void checkTypesCompatible(const Types& _output_types, const Types& _input_types);

        private:
            Any value_;

            Types input_types_;
            Types output_types_;

            const TypesConverter& converter_;
    };

} // end namespace BT

#endif   // BLACKBOARD_ENTRY_H
