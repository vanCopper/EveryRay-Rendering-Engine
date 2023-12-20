#pragma once
#include <cstdint>

namespace EveryRay_Core
{
    struct RenderGraphResourceName
    {
        static constexpr uint64_t INVALID_HASH = uint64_t(-1);

        RenderGraphResourceName() : hashed_name(INVALID_HASH), name{ "uninitialized" } {}

        template<uint64_t N>
        constexpr explicit RenderGraphResourceName(char const(&name)[N], uint64_t hash) : hashed_name(hash), name(name)
        {}

        operator char const* () const
        {
            return name;
        }

        bool IsValidName() const
        {
            return hashed_name != INVALID_HASH;
        }
        
        uint64_t hashed_name;
        char const* name;
    };
    
    using RGResourceName = RenderGraphResourceName;
    inline bool operator==(RenderGraphResourceName const& name1, RenderGraphResourceName const& name2)
    {
        return name1.hashed_name == name2.hashed_name;
    }
}

namespace std
{
    template<> struct hash<EveryRay_Core::RenderGraphResourceName>
    {
        size_t operator()(EveryRay_Core::RenderGraphResourceName const& resource_name) const noexcept
        {
            return hash<decltype(resource_name.hashed_name)>()(resource_name.hashed_name);
        }
    };
}
