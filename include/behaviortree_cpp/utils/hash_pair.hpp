#ifndef HASH_PAIR_HPP
#define HASH_PAIR_HPP

#include <functional>
#include <utility>

namespace BT
{

// Unashamedly stolen from https://stackoverflow.com/a/55083395
struct hash_pair final
{
    template<class TFirst, class TSecond>
    size_t operator()(const std::pair<TFirst, TSecond>& p) const noexcept
    {
        uintmax_t hash = std::hash<TFirst>{}(p.first);
        hash <<= sizeof(uintmax_t) * 4;
        hash ^= std::hash<TSecond>{}(p.second);

        return std::hash<uintmax_t>{}(hash);
    }
};

}

#endif // HASH_PAIR_HPP
