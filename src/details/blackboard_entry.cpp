#include "behaviortree_cpp/details/blackboard_entry.h"

namespace BT
{
namespace details
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
        AddPortType(value_.type(), _port_info.direction());
    }

    Entry::Entry(Any&& other_any, const PortDirection _direction, const TypesConverter& _converter):
      value_     (std::move(other_any)),
      converter_ (_converter)
    {
        AddPortType(value_.type(), _direction);
    }

    void Entry:set(Any&& _other_any)
    {
        value_ = _other_any;
        AddType(value_.type(), PortDirection::INPUT);
    }

    const Types& input_types  const
    {
        return input_types_;
    }

    const Types& output_types const
    {
        return output_types_;
    }

    void addPortType(const PortDirection _port_info)
    {
    }

    void addPortType(const PortDirection _port_info)
    {
        std::type_index port_type = *_port_info.type();

        switch(_port_info.direction())
        {
            case PortInfo::INPUT:
                input_types.insert(port_type);
                break;
            case PortInfo::OUTPUT:
                output_types.insert(port_type);
                break;
            case PortInfo::INOUT:
                input_types.insert(port_type);
                output_types.insert(port_type);
                break;
        }
    }

    // Private

} // end namespace details
} // end namespace BT
