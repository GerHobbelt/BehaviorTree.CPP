#ifndef GET_LINUX_ENV_VAR_NODE_HPP
#define GET_LINUX_ENV_VAR_NODE_HPP

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
class GetLinuxEnvVarNode final : public BT::SyncActionNode
{
    public:
        using BT::SyncActionNode::SyncActionNode;
        ~GetLinuxEnvVarNode() = default;

        static BT::PortsList providedPorts()
        {
            return { BT::InputPort<std::string>("env_var_name", "Name of the environment variable to get"),
                     BT::OutputPort<std::string>("output", "Value of the environment variable")
                   };
        }

        virtual BT::NodeStatus tick() override
        {
            const auto& env_var_name = getInput<std::string>("env_var_name");

            if(!env_var_name) { throw BT::RuntimeError { name() + ": " + env_var_name.error() }; }

            char * val;

            if(env_var_name.value() != "HOSTNAME")
                val = getenv(env_var_name.value().c_str());
            else
                gethostname(val, HOST_NAME_MAX + 1);

            std::string output_string = val == NULL ? std::string("") : std::string(val);

            setOutput("output", output_string);
            return output_string == "" ? BT::NodeStatus::FAILURE : BT::NodeStatus::SUCCESS;
        }

}; // class GetLinuxEnvVarNode
} // namespace BT

#endif // GET_LINUX_ENV_VAR_NODE_HPP