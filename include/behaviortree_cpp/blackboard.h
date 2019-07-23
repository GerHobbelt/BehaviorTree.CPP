#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/utils/types_converter.hpp"
#include "behaviortree_cpp/exceptions.h"

namespace BT
{

/**
 * @brief The Blackboard is the mechanism used by BehaviorTrees to exchange
 * typed data.
 */
class Blackboard
{
  public:
    typedef std::shared_ptr<Blackboard> Ptr;

  protected:
    // This is intentionally protected. Use Blackboard::create instead
    Blackboard(Blackboard::Ptr parent): parent_bb_(parent)
    {}

  public:

    /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
    static Blackboard::Ptr create(Blackboard::Ptr parent = {})
    {
        return std::shared_ptr<Blackboard>( new Blackboard(parent) );
    }

    virtual ~Blackboard() = default;

    /**
     * @brief The method getAny allow the user to access directly the type
     * erased value.
     *
     * @return the pointer or nullptr if it fails.
     */
    const Any* getAny(const std::string& key) const
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find(key);
            if( remapping_it != internal_to_external_.end())
            {
                return parent->getAny( remapping_it->second );
            }
        }
        auto it = storage_.find(key);
        return ( it == storage_.end()) ? nullptr : &(it->second.value);
    }

    Any* getAny(const std::string& key)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find(key);
            if( remapping_it != internal_to_external_.end())
            {
                return parent->getAny( remapping_it->second );
            }
        }
        auto it = storage_.find(key);
        return ( it == storage_.end()) ? nullptr : &(it->second.value);
    }

    /** Return true if the entry with the given key was found.
     *  Note that this method may throw an exception if the cast to T failed.
     */
    template <typename T>
    bool get(const std::string& key, T& value) const
    {
        const Any* val = getAny(key);
        if (val)
        {
            value = val->cast<T>();
        }
        return (bool)val;
    }

    /**
     * Version of get() that throws if it fails.
    */
    template <typename T>
    T get(const std::string& key) const
    {
        const Any* val = getAny(key);
        if (val)
        {
            return val->cast<T>();
        }
        else
        {
            throw RuntimeError("Blackboard::get() error. Missing key [", key, "]");
        }
    }


    /// Update the entry with the given key
    template <typename T>
    void set(const std::string& key, const T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = storage_.find(key);

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find(key);
            if( remapping_it != internal_to_external_.end())
            {
                const auto& remapped_key = remapping_it->second;
                if( it == storage_.end() ) // virgin entry
                {
                    auto parent_info = parent->portInfo(remapped_key);
                    if( parent_info )
                    {
                        storage_.insert( {key, Entry( *parent_info ) } );
                    }
                    else{
                        storage_.insert( {key, Entry( PortInfo() ) } );
                    }
                }
                parent->set( remapped_key, value );
                return;
            }
        }

        if( it != storage_.end() ) // already there. check the type
        {
            it->second.value = Any(value);
            /*
            const PortInfo& port_info = it->second.port_info;
            auto& previous_any = it->second.value;
            const auto locked_type = port_info.type();

            //NOTE: type checks disabled on set
            Any temp(value);
            if( locked_type && locked_type != &typeid(T) && locked_type != &temp.type() )
            {
                bool mismatching = true;
                if( std::is_constructible<StringView, T>::value )
                {
                    Any any_from_string = port_info.parseString( value );
                    if( any_from_string.empty() == false)
                    {
                        mismatching = false;
                        temp = std::move( any_from_string );
                    }
                }
                else if(types_converter_ && types_converter_.value().isConvertible(typeid(T), *locked_type))
                {
                    std::cout << "Blacboard::set() detected convertible types" << std::endl;
                    mismatching = false;
                }
                else if(temp.isNumber() && previous_any.isNumber())
                {
                    std::cout << "Temp with type " << demangle(temp.type()) << " is number " << temp.isNumber() << std::endl;
                    std::cout << "Previous with type " << demangle(previous_any.type()) << " is number " << temp.isNumber() << std::endl;
                    std::cout << "--------------" << std::endl;
                    mismatching = false;
                }

                if( mismatching )
                {
                    debugMessage();

                    std::cout << "MISMATCH Temp with type " << demangle(temp.type()) << " is number " << temp.isNumber() << std::endl;
                    std::cout << "MISMATCH Previous with type " << demangle(previous_any.type()) << " is number " << previous_any.isNumber() << std::endl;
                    std::cout << "--------------" << std::endl;

                    throw LogicError( "Blackboard.h Blackboard::set() failed: once declared, the type of a port shall not change. "
                                     "Declared type [", demangle( locked_type ),
                                     "] != current type [", demangle( typeid(T) ),"]" );
                }
            }
            previous_any = std::move(temp);
            */
        }
        else
        {
            // create for the first time without any info
            storage_.emplace( key, Entry( Any(value), PortInfo() ) );
        }
        return;
    }

    void setPortInfo(std::string key, const PortInfo& info);
    void addPortType(std::string key, const std::type_info* type);

    void setTypesConverter(const TypesConverter& types_converter);

    const PortInfo *portInfo(const std::string& key);

    void addSubtreeRemapping(std::string internal, std::string external);

    void debugMessage() const;

    //TODO: should this be an unique global reference for all the trees?
    Optional<TypesConverter> types_converter_;

  private:
    bool areEntryTypesCompatible(const Entry& _entry);

  private:
    struct Entry
    {
        public:
            using Types = std::set<std::type_index>;

        public:
            Entry( const PortInfo& _port_info)
            {
                AddPortType(_port_info);
            }

            Entry(Any&& other_any, const PortInfo& _port_info):
              value (std::move(other_any))
            {
                AddPortType(_port_info);
            }

            Any& value() { return value_; }
            const Types& input_types  const { return input_types_;  }
            const Types& output_types const { return output_types_; }

            void addPortType(const PortInfo& _port_info)
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

        private:
            Any value_;
            Types input_types_;
            Types output_types_;
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Entry> storage_;
    std::weak_ptr<Blackboard> parent_bb_;
    std::unordered_map<std::string,std::string> internal_to_external_;
};

} // end namespace

#endif   // BLACKBOARD_H
