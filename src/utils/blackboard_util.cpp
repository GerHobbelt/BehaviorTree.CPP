#include "behaviortree_cpp_v3/utils/blackboard_util.h"

namespace BT
{

bool isBlackboardPointer(StringView str)
{
    auto size = str.size();
    if( size >= 3)
    {
        while ( str.starts_with(' ') ) {str = str.substr(1, size-1); size = str.size();}
        while ( str.ends_with(' ') ) {str = str.substr(0, size-1); size = str.size();}

        if( str[0] == '{' && str.back() == '}') {
            return true;
        }
        if( size >= 4 && str[0] == '$' && str[1] == '{' && str.back() == '}') {
            return true;
        }
    }
    return false;
}

StringView stripBlackboardPointer(StringView str)
{
    auto size = str.size();
    if( size >= 3 )
    {
        while ( str.starts_with(' ') ) {str = str.substr(1, size-1); size = str.size();}
        while ( str.ends_with(' ') ) {str = str.substr(0, size-1); size = str.size();}

        if( str[0] == '{' && str.back() == '}') {
            return str.substr(1, size-2);
        }
        if( str[0] == '$' && str[1] == '{' && str.back() == '}') {
            return str.substr(2, size-3);
        }
    }
    return {};
}

}   // end namespace
