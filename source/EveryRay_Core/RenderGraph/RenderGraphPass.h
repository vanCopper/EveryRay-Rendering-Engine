#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "RenderGraphResourceId.h"

enum class ER_RHI_RESOURCE_STATE;

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
    
    class RenderGraph;
    class RenderGraphBuilder;
    
    class RenderGraphPassBase
    {
        friend RenderGraph;
        // friend RenderGraphBuilder;

        struct RenderTargetInfo
        {
            RGRenderTargetId render_target_handle;
            RGLoadStoreAccessOp render_target_access;
        };

        struct DepthStencilInfo
        {
            RGDepthStencilId depth_stencil_handle;
            RGLoadStoreAccessOp depth_access;
            RGLoadStoreAccessOp stencil_access;
            bool depth_read_only;
        };

       static uint32_t unique_pass_id;
        
    public:
        explicit RenderGraphPassBase(char const* name, RGPassType type = RGPassType::Graphics, RGPassFlags flags = RGPassFlags::None)
            : name(name), type(type), flags(flags){}
        virtual ~RenderGraphPassBase() = default;

    private:
        std::string const name;
        uint64_t ref_count = 0;
        RGPassType type;
        RGPassFlags flags = RGPassFlags::None;
        uint64_t id;

        std::unordered_set<RGTextureId> texture_creates;
        std::unordered_set<RGTextureId> texture_reads;
        std::unordered_set<RGTextureId> texture_writes;
        std::unordered_set<RGTextureId> texture_destroys;
        std::unordered_map<RGTextureId, ER_RHI_RESOURCE_STATE> texture_state_map;

        std::unordered_set<RGBufferId> buffer_creates;
        std::unordered_set<RGBufferId> buffer_reads;
        std::unordered_set<RGBufferId> buffer_writes;
        std::unordered_set<RGBufferId> buffer_destroys;
        std::unordered_map<RGBufferId, ER_RHI_RESOURCE_STATE> buffer_state_map;

        std::vector<RenderTargetInfo> render_targets_info;
        std::unique_ptr<DepthStencilInfo> depth_stencil = nullptr;
        uint32_t viewport_width = 0;
        uint32_t viewport_height = 0;
    };
    using RGPassBase = RenderGraphPassBase;

    template <typename PassData>
    class RenderGraphPass final : public RenderGraphPassBase
    {
    };
}
