#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "behaviortree_cpp/blackboard_entry.h"
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
     * Version of get() that returns an expected
    */
    template <typename T>
    Optional<T> get(const std::string& key)
    {
        Optional<Entry> expected_entry = getEntry(key);

        if(!expected_entry) { return nonstd::make_unexpected(expected_entry.error()); }
        return expected_entry.value().getValue<T>();
    }

    // Update the entry with the given key
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
                    const Optional<Entry>& parent_entry = parent->getEntry(remapped_key);
                    if( parent_entry )
                    {
                        storage_.insert( { key, Entry( parent_entry.value(), types_converter_) } );
                    }
                    else
                    {
                        storage_.insert( {key, Entry( PortInfo(), types_converter_ ) } );
                    }
                }

                parent->set( remapped_key, value );
                return;
            }
        }

        // Update it or create it for the first time 
        if( it != storage_.end() )
        {
            it->second.setValue(value);
        }
        else
        {
            storage_.emplace( key, Entry( Any(value), PortDirection::OUTPUT, types_converter_ ) );
        }
    }

    void setPortInfo(std::string key, const PortInfo& info);

    void setTypesConverter(const TypesConverter& types_converter);

    void addSubtreeRemapping(std::string internal, std::string external);

    void debugMessage() const;

  private:
    Optional<Entry> getEntry(const std::string& key) const;

  private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Entry> storage_;
    std::weak_ptr<Blackboard> parent_bb_;
    std::unordered_map<std::string,std::string> internal_to_external_;

    //TODO: should this be an unique global reference for all the trees?
    TypesConverter types_converter_;
};

} // end namespace

#endif   // BLACKBOARD_H
