#ifndef SUBSTRING_NODE_HPP
#define SUBSTRING_NODE_HPP

#include "behaviortree_cpp/action_node.h"

namespace BT
{
class SubstringNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~SubstringNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("input", "String to look into"),
                     //BT::InputPort<std::vector<std::string>>("substrings", "Substrings to look for"), // It doesn't allow a ${var} value, that is a std::string
                     BT::InputPort<std::string>("substrings", "Substrings to look for"), // Receiving "substring1;substring2;substring3" as string ...
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            setStatus(BT::NodeStatus::RUNNING);

            const auto& input      = getInput<std::string>("input");
            //const auto& substrings = getInput<std::vector<std::string>>("substrings");
            const auto& substrings_str = getInput<std::string>("substrings"); 

            if(!input)      { throw BT::RuntimeError { name() + ": " + input.error()  }; }
            if(!substrings_str) { throw BT::RuntimeError { name() + ": " + substrings_str.error() }; }

            // ... and splitting it within a std::vector 
            std::vector<std::string> substrings;
            std::string::size_type prev_pos = 0, pos = 0;
            while((pos = substrings_str.value().find(";", pos)) != std::string::npos)
            {
                std::string substring_tmp( substrings_str.value().substr(prev_pos, pos-prev_pos) );
                substrings.push_back(substring_tmp);
                prev_pos = ++pos;
            }
            substrings.push_back(substrings_str.value().substr(prev_pos, pos-prev_pos)); // last substring
            
            for(const auto& substring : substrings)
            {
                if(input.value().find(substring) != std::string::npos) { return BT::NodeStatus::SUCCESS; }
            }

            return BT::NodeStatus::FAILURE;
        }
};
}

#endif
