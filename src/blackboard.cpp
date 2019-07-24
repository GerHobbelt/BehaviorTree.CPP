#include "behaviortree_cpp/blackboard.h"

namespace BT{

/*
void Blackboard::setPortInfo(std::string key, const PortInfo& info)
{
    std::cout << "Blackboard setPortInfo. Key: " << key << std::endl;

    std::unique_lock<std::mutex> lock(mutex_);

    if( auto parent = parent_bb_.lock())
    {
        auto remapping_it = internal_to_external_.find(key);
        if( remapping_it != internal_to_external_.end())
        {
            parent->setPortInfo( remapping_it->second, info );
        }
    }

    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        storage_.insert( { std::move(key), Entry(info) } );
    }
    else{
        auto old_type = it->second.port_info.type();
        if( old_type && old_type != info.type() )
        {
            throw LogicError( " This happened Blackboard::set() failed: once declared, the type of a port shall not change. "
                             "Declared type [",     BT::demangle( old_type ),
                             "] != current type [", BT::demangle( info.type() ), "]" );
        }
    }
}
*/

void Blackboard::addPortType(std::string key, const std::type_info* info)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if( auto parent = parent_bb_.lock())
    {
        auto remapping_it = internal_to_external_.find(key);
        if( remapping_it != internal_to_external_.end())
        {
            parent->addPortType( remapping_it->second, info );
        }
    }

    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        storage_.insert( { std::move(key), Entry(*info) } );
    }
}

/*
const PortInfo* Blackboard::portInfo(const std::string &key)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        return nullptr;
    }
    return &(it->second.port_info);
}
*/

void Blackboard::addSubtreeRemapping(std::string internal, std::string external)
{
    internal_to_external_.insert( {std::move(internal), std::move(external)} );
}

void Blackboard::setTypesConverter(const TypesConverter& types_converter)
{
    types_converter_ = types_converter;
}

void Blackboard::debugMessage() const
{
    for(const auto& entry_it: storage_)
    {
        auto port_type = entry_it.second.port_info.type();
        if( !port_type )
        {
            port_type = &( entry_it.second.value.type() );
        }

        std::cout <<  entry_it.first << " (" << demangle( port_type ) << ") -> ";

        if( auto parent = parent_bb_.lock())
        {
            auto remapping_it = internal_to_external_.find( entry_it.first );
            if( remapping_it != internal_to_external_.end())
            {
                std::cout << "remapped to parent [" << remapping_it->second << "]" <<std::endl;
                continue;
            }
        }

        std::cout << ((entry_it.second.value.empty()) ? "empty" : "full") <<  std::endl;
    }
}

// Private
bool Blackboard::areEntryTypesCompatible(const Entry& _entry)
{
    const Entry::Types& input_types = _entry.input_types();

    for(const std::type_index& output_type : _entry.output_types())
    {
        if(!std::all_of(input_types.cbegin(), input_types.cend(),
                    [&output_type] (const std::type_index& _input_type)
                    { return types_converter_.value().isConvertible(output_type, _input_type) }))
        {
            return false;
        }
    }

    return true;
}

Expected<Entry> Blackboard::getEntry(const std::string& key)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if( auto parent = parent_bb_.lock())
    {
        auto remapping_it = internal_to_external_.find(key);
        if( remapping_it != internal_to_external_.end())
        {
            return parent->getEntry( remapping_it->second );
        }
    }

    auto it = storage_.find(key);
    return ( it == storage_.end()) ? nonstd::make_unexpected { "Key ", key, " not found" } : *it;
}

}
}
