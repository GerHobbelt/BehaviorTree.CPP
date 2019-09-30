#ifndef TYPES_CONVERTER_IMPL_HPP
#define TYPES_CONVERTER_IMPL_HPP

#include <set>

#include "behaviortree_cpp/utils/types_converter.hpp"

namespace BT
{
    template<class From, class To>
    void TypesConverter::addConversion(const ConverterFunction<From, To>& converter)
    {
        const TypeKey& type_key = getTypeKey(typeid(From), typeid(To));

        const auto converter_any = [converter] (const Any& from_any)
                                   {
                                       return Any(converter(from_any.cast<From>()));
                                   };

        converters_[type_key] = converter_any;
    }

    template<class From, class To>
    To TypesConverter::convert(From value) const
    {
        return convert<To>(value);
    }

    template<class To>
    To TypesConverter::convert(Any value) const
    {
        if(value.empty())
        {
            throw std::runtime_error { "TypesConverter::convert failed because value is empty" };
        }

        if(value.isConvertible<To>()) { return value.cast<To>(); }

        const AnyConverter& conversion = getConversion<To>(value.type());
        const Any& converted_any       = conversion(value);

        return converted_any.cast<To>();
    }

    template<class From, class To>
    bool TypesConverter::isConvertible() const
    {
        return isConvertible(typeid(From), typeid(To));
    }

    template<class To>
    bool TypesConverter::isConvertible(const std::type_info& from_type) const
    {
        return isConvertible(from_type, typeid(To));
    }

    inline bool TypesConverter::isConvertible(const std::type_info& from_type, const std::type_info& to_type) const
    {
        return isConvertible(std::type_index(from_type), std::type_index(to_type));
    }

    inline bool TypesConverter::isConvertible(const std::type_index& from_type, const std::type_index& to_type) const
    {
        //Check first if a basic conversion is possible
        if( (from_type == to_type) ||
            (from_type == typeid(std::string) && isBasicType(to_type)) ||
            (isBasicType(from_type) && isBasicType(to_type)))
        { return true; }

        //Look for a user-defined conversion otherwise
        const TypeKey type_key { from_type, to_type };
        return converters_.find(type_key) != converters_.cend();
    }

    // Private
    inline TypesConverter::TypeKey TypesConverter::getTypeKey(const std::type_info& from_type, const std::type_info& to_type) const
    {
        return { from_type, to_type };
    }

    template<class From, class To>
    TypesConverter::AnyConverter TypesConverter::getConversion() const
    {
        return getConversion(typeid(From), typeid(To));
    }

    template<class To>
    TypesConverter::AnyConverter TypesConverter::getConversion(const std::type_info& _from) const
    {
        return getConversion(_from, typeid(To));
    }

    inline TypesConverter::AnyConverter TypesConverter::getConversion(const std::type_info& _from, const std::type_info& _to) const
    {
        try
        {
            const TypeKey& key = getTypeKey(_from, _to);
            return converters_.at(key);
        }
        catch(const std::out_of_range&)
        {
            throw LogicError { "Type conversion failed. No known conversion from type ", demangle(_from),
                                " to type ", demangle(_to) };
        }
    }

    inline bool TypesConverter::isBasicType(const std::type_index& _type) const
    {
        //These are all the types (except enums) that are supported by SafeAny by default
        //Note: this does not detect enums as a convertible type (it's not possible to detect a family of types
        //just with runtime info as typeid)
        static const std::set<std::type_index> convertible_types
        {
            typeid(bool),    typeid(char),              typeid(char16_t),    typeid(char32_t),  typeid(wchar_t),
            typeid(unsigned char),    typeid(unsigned), typeid(uint16_t),    typeid(uint32_t),  typeid(uint64_t),
            typeid(short),   typeid(int),               typeid(long),        typeid(long long), typeid(float),
            typeid(double),  typeid(long double),       typeid(std::string), typeid(SafeAny::SimpleString)
        };

        return convertible_types.find(_type) != convertible_types.cend();
    }
}

#endif   // TYPES_CONVERTER_IMPL_HPP
