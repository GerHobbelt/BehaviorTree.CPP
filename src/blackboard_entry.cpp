#include "behaviortree_cpp/blackboard_entry.h"

namespace BT
{
    Entry::Entry(const PortInfo& _port_info, const TypesConverter& _converter) :
        converter_ ( _converter )
    {
        AddType(_port_info);
    }

    Entry::Entry(Any&& other_any, const PortInfo& _port_info, const TypesConverter& _converter):
      value_     (std::move(other_any)),
      converter_ (_converter)
    {
        AddType(value_.type(), _port_info.direction());
    }

    Entry::Entry(Any&& other_any, const PortDirection _direction, const TypesConverter& _converter):
      value_     (std::move(other_any)),
      converter_ (_converter)
    {
        AddType(value_.type(), _direction);
    }

    template<>
    void Entry:set(Any&& _other_any)
    {
        AddType(_other_any.type(), PortDirection::OUTPUT);
        value_ = _other_any;
    }

    template<>
    Any Entry:get() const
    {
        return value_;
    }

    void addType(const PortInfo& _port_info)
    {
        addType(_port_info.type(), _port_info.direction());
    }

    void addType(const std::type_info* _type, const PortDirection _port_info)
    {
        std::pair<Types::iterator, bool> insert_result;
        std::type_index port_type = *_port_info.type();

        switch(_port_info.direction())
        {
            case PortInfo::INPUT:
                insert_result = input_types_.insert(port_type);
                break;
            case PortInfo::OUTPUT:
                insert_result = output_types_.insert(port_type);
                break;
            case PortInfo::INOUT:
                //TODO: fix this comparison
                insert_result = input_types_.insert(port_type);
                insert_result = output_types_.insert(port_type);
                break;
        }

        //Check for type compatibility only if there has been changes in the type
        if(insert_result.second)
        {
            checkTypesCompatible(output_types_, input_types_);
        }
    }

    // Private
    void Entry::checkTypesCompatible(const Types& _output_types, const Types& _input_types)
    {
        for(const std::type_index& output_type : _output_types)
        {
            for(const std::type_index& input_type : _input_types)
            {
                if(!converter_.isConvertible(output_type, input_type))
                {
                    throw LogicError { "Incompatible entry types. No known conversion between",
                                        demangle(output_type), " to ", demangle(input_type) };
                }
            }
        }
    }

} // end namespace BT
