#include "behaviortree_cpp/blackboard_entry.h"

namespace BT
{
    Entry::Entry(const Entry& _other_entry, const TypesConverter& _converter) :
        converter_ ( _converter )
    {
        input_types_  = _other_entry.input_types_;
        output_types_ = _other_entry.output_types_;

        checkTypesCompatible(input_types_, output_types_);
    }

    Entry::Entry(const PortInfo& _port_info, const TypesConverter& _converter) :
        converter_ ( _converter )
    {
        addPortInfo(_port_info);
    }

    Entry::Entry(Any&& other_any, const PortInfo& _port_info, const TypesConverter& _converter):
      value_     (std::move(other_any)),
      converter_ (_converter)
    {
        addPortInfo(value_.type(), _port_info.direction());
    }

    Entry::Entry(Any&& other_any, const PortDirection _direction, const TypesConverter& _converter):
      value_     (std::move(other_any)),
      converter_ (_converter)
    {
        addPortInfo(value_.type(), _direction);
    }

    template<>
    void Entry::setValue(Any&& _other_any)
    {
        addPortInfo(_other_any.type(), PortDirection::OUTPUT);
        value_ = _other_any;
    }

    template<>
    Any Entry::getValue()
    {
        return value_;
    }

    void Entry::addPortInfo(const PortInfo& _port_info)
    {
        if(!_port_info.type()) { return; }

        addPortInfo(*_port_info.type(), _port_info.direction());
    }

    void Entry::addPortInfo(const std::type_info& _type, const PortDirection _direction)
    {
        std::pair<Types::iterator, bool> insert_result;

        switch(_direction)
        {
            case PortDirection::INPUT:
                insert_result = input_types_.insert(_type);
                break;
            case PortDirection::OUTPUT:
                insert_result = output_types_.insert(_type);
                break;
            case PortDirection::INOUT:
                //TODO: fix this comparison
                insert_result = input_types_.insert(_type);
                insert_result = output_types_.insert(_type);
                break;
        }

        std::cout << "BEFORE CHECK TYPES COMPATIBLES" << std::endl;

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
                std::cout << "Comparing output type " << demangle(output_type)
                    << " with input type " << demangle(input_type) << std::endl;
                if(!converter_.isConvertible(output_type, input_type))
                {
                    throw LogicError { "Incompatible entry types. No known conversion from ",
                                        demangle(output_type), " to ", demangle(input_type) };
                }
            }
        }
    }

} // end namespace BT
