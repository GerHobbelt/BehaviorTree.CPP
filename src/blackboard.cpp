#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/utils/blackboard_util.h"

namespace BT
{

void Blackboard::setPortInfo(std::string key, const PortInfo& info)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if( auto parent = parent_bb_.lock())
    {
        auto remapping_it = internal_to_external_.find(key);
        if( remapping_it != internal_to_external_.end())
        {
            std::string port_key;

            // Only set the port info if the external port is a blackboard pointer
            // If it's not a pointer, it means is a literal value written in the node's port
            // and it shouldn't be stored in the blackboard, since the name could collide
            // with other entries
            if(isBlackboardPointer(remapping_it->second))
            {
                parent->setPortInfo( std::string(stripBlackboardPointer(remapping_it->second)), info );
            }
        }
    }

    auto it = storage_.find(key);
    if( it == storage_.end() )
    {
        storage_.insert( { std::move(key), Entry(info, types_converter_) } );
    }
    else
    {
        it->second.addPortInfo(info);
    }
}

void Blackboard::addSubtreeRemapping(std::string internal, std::string external)
{
    internal_to_external_.insert( {std::move(internal), std::move(external)} );
}

void Blackboard::setTypesConverter(const TypesConverter& types_converter)
{
    types_converter_ = types_converter;
}

//TODO: re-implement this
void Blackboard::debugMessage() const
{
    std::cout << "Blackboard::debugMessage not yet implemented" << std::endl;

    /*
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
    */
}

// Private
Optional<Entry> Blackboard::getEntry(const std::string& key) const
{
    std::unique_lock<std::mutex> lock(mutex_);

    if( auto parent = parent_bb_.lock())
    {
        auto remapping_it = internal_to_external_.find(key);
        if( remapping_it != internal_to_external_.end())
        {
            if(isBlackboardPointer(remapping_it->second))
            {
                return parent->getEntry( std::string(stripBlackboardPointer(remapping_it->second)) );
            }

            // TODO: this probably should be moved from here, and call the types_converter_
            // function directly instead of creating a new virtual entry
            // (this case is a literal value written in the port using the GUI)
            return Entry(Any(remapping_it->second), types_converter_);
        }
    }

    auto it = storage_.find(key);
    return ( it == storage_.end()) ? nonstd::make_unexpected(StrCat("Key ", key, " not found")) :  Optional<Entry>(it->second);
}

}
