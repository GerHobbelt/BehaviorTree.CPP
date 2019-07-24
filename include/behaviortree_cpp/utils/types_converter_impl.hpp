#ifndef TYPES_CONVERTER_IMPL_HPP
#define TYPES_CONVERTER_IMPL_HPP

#include "behaviortree_cpp/utils/types_converter.hpp"

namespace BT
{
    template<class From, class To>
    void TypesConverter::addConversion(const ConverterFunction<From, To>& converter)
    {
        const TypeKey& type_key = getTypeKey<From, To>();

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
        const TypeKey& type_key  = getTypeKey<To>(value.type());
        const ConverterFunction& conversion = getConversion(type_key);
        const Any& converted_any& = conversion(value);

        return converted_any.cast<To>();
    }

    template<class To>
    To TypesConverter::convert(const Any* value) const
    {
        const TypeKey& type_key  = getTypeKey<To>(value->type());
        const ConverterFunction& conversion = getConversion(type_key);
        const Any converted_any& = conversion(*value);

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
        if(from_type == to_type) { return true; }

        const TypeKey& type_key = getTypeKey(from_type, to_type);
        return converters_.find(type_key) != converters_.cend();
    }

    template<class From, class To>
    TypesConverter::TypeKey TypesConverter::getTypeKey() const
    {
        return getTypeKey(typeid(From), typeid(To));
    }

    template<class To>
    TypesConverter::TypeKey TypesConverter::getTypeKey(const std::type_info& from_type) const
    {
        return getTypeKey(from_type, typeid(To));
    }

    inline TypesConverter::TypeKey TypesConverter::getTypeKey(const std::type_info& from_type, const std::type_info& to_type) const
    {
        return { from_type, to_type };
    }

    inline TypesConverter::ConverterFunction getConversion(const TypeKey& _key) const
    {
        try
        {
            return converters_.at(_key);
        }
        catch(const std::out_of_range&)
        {
            throw LogicError { "Type conversion failed. No known conversion from type ", demangle(_key.first),
                                " to type ", demangle(_key.second) };
        }
    }
}

#endif   // TYPES_CONVERTER_IMPL_HPP
