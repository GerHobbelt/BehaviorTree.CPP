#ifndef TYPES_CONVERTER_HPP
#define TYPES_CONVERTER_HPP

#include <typeindex>
#include <unordered_map>

#include "behaviortree_cpp/utils/hash_pair.hpp"
#include "behaviortree_cpp/utils/safe_any.hpp"

namespace BT
{
    template <class From, class To>
    using ConverterFunction = std::function<To(From)>;

    class TypesConverter final
    {
        public:
            ~TypesConverter() = default;

            template<class From, class To>
            void addConversion(const ConverterFunction<From, To>& converter);

            template<class From, class To>
            To convert(From value) const;

            template<class To>
            To convert(Any value) const;

            template<class To>
            To convert(const Any* value) const;

            template<class From, class To>
            bool isConvertible() const;

            template<class To>
            bool isConvertible(const std::type_info& from_type) const;

            bool isConvertible(const std::type_info& from_type, const std::type_info& to_type) const;

        private:
            using TypeKey = std::pair<std::type_index, std::type_index>;

            template<class From, class To>
            TypeKey getTypeKey() const;

            template<class To>
            TypeKey getTypeKey(const std::type_info& from_type) const;

            TypeKey getTypeKey(const std::type_info& from_type, const std::type_info& to_type) const;

        private:
            using AnyConverter = std::function<Any(const Any&)>;
            std::unordered_map<TypeKey, AnyConverter, hash_pair> converters_;
    };
}

#include "behaviortree_cpp/utils/types_converter_impl.hpp"

#endif   // TYPES_CONVERTER_HPP
