#pragma once
#include <cstdint>

namespace EveryRay_Core
{
    enum class RGPassType : uint8_t
    {
        Graphics,
        Compute,
        ComputeAsync,
        Copy
    };

    enum class RGPassFlags : uint32_t
    {
        None = 0x00,
        ForceNoCull = 0x01,
        SkipAutoRenderPass = 0x02,
        LegacyRenderPass = 0x04,
        AllowUAVWrites = 0x08
    };

    enum class RGReadAccess : uint8_t
    {
        ReadAccess_PixelShader,
        ReadAccess_NonPixelShader,
        ReadAccess_AllShader
    };

    enum class RGLoadAccessOp : uint8_t
    {
        Discard,
        Preserve,
        Clear,
        NoAccess
    };

    enum class RGStoreAccessOp : uint8_t
    {
        Discard,
        Preserve,
        Resolve,
        NoAccess
    };

    inline constexpr uint8_t CombineAccessOps(RGLoadAccessOp load_op, RGStoreAccessOp store_op)
    {
        return (uint8_t)load_op << 2 | (uint8_t)store_op;
    }

    enum class RGLoadStoreAccessOp : uint8_t
    {
        Discard_Discard = CombineAccessOps(RGLoadAccessOp::Discard, RGStoreAccessOp::Discard),
        Discard_Preserve = CombineAccessOps(RGLoadAccessOp::Discard, RGStoreAccessOp::Preserve),
        Clear_Preserve = CombineAccessOps(RGLoadAccessOp::Clear, RGStoreAccessOp::Preserve),
        Preserve_Preserve = CombineAccessOps(RGLoadAccessOp::Preserve, RGStoreAccessOp::Preserve),
        Clear_Discard = CombineAccessOps(RGLoadAccessOp::Clear, RGStoreAccessOp::Discard),
        Preserve_Discard = CombineAccessOps(RGLoadAccessOp::Preserve, RGStoreAccessOp::Discard),
        Clear_Resolve = CombineAccessOps(RGLoadAccessOp::Clear, RGStoreAccessOp::Resolve),
        Preserve_Resolve = CombineAccessOps(RGLoadAccessOp::Preserve, RGStoreAccessOp::Resolve),
        Discard_Resolve = CombineAccessOps(RGLoadAccessOp::Discard, RGStoreAccessOp::Resolve),
        NoAccess_NoAccess = CombineAccessOps(RGLoadAccessOp::NoAccess, RGStoreAccessOp::NoAccess),
    };

    inline constexpr void SplitAccessOp(RGLoadStoreAccessOp load_store_op, RGLoadAccessOp& load_op, RGStoreAccessOp& store_op)
    {
        store_op = static_cast<RGStoreAccessOp>((uint8_t)load_store_op & 0b11);
        load_op = static_cast<RGLoadAccessOp>(((uint8_t)load_store_op >> 2) & 0b11);
    }
    
    
    class RenderGraphPass
    {
    public:
    
    };
}
