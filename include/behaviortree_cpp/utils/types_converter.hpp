#ifndef TYPES_CONVERTER_HPP
#define TYPES_CONVERTER_HPP

#include <typeindex>
#include <unordered_map>

#include "behaviortree_cpp/utils/hash_pair.hpp"
#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/exceptions.h"

namespace BT
{
    template <class From, class To>
    using ConverterFunction = std::function<To(From)>;

    class TypesConverter final
    {
        private:
            using TypeKey      = std::pair<std::type_index, std::type_index>;
            using AnyConverter = std::function<Any(const Any&)>;

        public:
            ~TypesConverter() = default;

            template<class From, class To>
            void addConversion(const ConverterFunction<From, To>& converter);

            template<class From, class To>
            To convert(From value) const;

            template<class To>
            To convert(Any value) const;

            template<class From, class To>
            bool isConvertible() const;

            template<class To>
            bool isConvertible(const std::type_info& from_type) const;

            bool isConvertible(const std::type_info& from_type,  const std::type_info& to_type) const;
            bool isConvertible(const std::type_index& from_type, const std::type_index& to_type) const;

        private:
            TypeKey getTypeKey(const std::type_info& from_type, const std::type_info& to_type) const;

            template<class From, class To>
            AnyConverter getConversion() const;

            template<class To>
            AnyConverter getConversion(const std::type_info& _from) const;

            AnyConverter getConversion(const std::type_info& _from, const std::type_info& _to) const;

            bool isBasicType(const std::type_index& _type) const;

        private:
            std::unordered_map<TypeKey, AnyConverter, hash_pair> converters_;
    };
}

#include "behaviortree_cpp/utils/types_converter_impl.hpp"

#endif   // TYPES_CONVERTER_HPP
