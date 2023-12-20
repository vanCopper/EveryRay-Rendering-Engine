#pragma once
#include <cstdint>

namespace EveryRay_Core
{
    enum class RGResourceType : uint8_t
    {
        Buffer,
        Texture,
    };

    enum class RGResourceMode : uint8_t
    {
        CopySrc,
        CopyDst,
        IndirectArgs,
        Vertex,
        Index,
        Constant
    };

    struct RenderGraphResourceId
    {
        constexpr static uint32_t invalid_id = uint32_t(-1);

        RenderGraphResourceId() : id(invalid_id) {}
        RenderGraphResourceId(RenderGraphResourceId const&) = default;
        explicit RenderGraphResourceId(uint64_t _id) : id(static_cast<uint32_t>(_id)) {}

        void Invalidate() { id = invalid_id; }
        bool IsValid() const { return id != invalid_id; }
        bool operator == (RenderGraphResourceId const& other) const { return this->id == other.id; }
        bool operator != (RenderGraphResourceId const& other) const { return this->id != other.id; }

        uint32_t id;
    };
    using RGResourceId = RenderGraphResourceId;


    template<RGResourceType ResourceType>
    struct TypedRenderGraphResourceId : RGResourceId
    {
        using RGResourceId::RGResourceId;
    };

    using RGBufferId = TypedRenderGraphResourceId<RGResourceType::Buffer>;
    using RGTextureId = TypedRenderGraphResourceId<RGResourceType::Texture>;

    template<RGResourceMode Mode>
    struct RGTextureModeId : RGTextureId
    {
        using RGTextureId::RGTextureId;
    private:
        // friend class RenderGraphBuilder;
        // friend class RenderGraph;
        RGTextureModeId(RGTextureId const& id) : RGTextureId(id) {}
    };

    template<RGResourceMode Mode>
    struct RGBufferModeId : RGBufferId
    {
        using RGBufferId::RGBufferId;
    private:
        // friend class RenderGraphBuilder;
        // friend class RenderGraph;
        RGBufferModeId(RGBufferId const& id) : RGBufferId(id) {}
    };

    using RGTextureCopySrcId = RGTextureModeId<RGResourceMode::CopySrc>;
    using RGTextureCopyDstId = RGTextureModeId<RGResourceMode::CopyDst>;

    using RGBufferCopySrcId = RGBufferModeId<RGResourceMode::CopySrc>;
    using RGBufferCopyDstId = RGBufferModeId<RGResourceMode::CopyDst>;
    using RGBufferIndirectArgsId = RGBufferModeId<RGResourceMode::IndirectArgs>;
    using RGBufferVertexId = RGBufferModeId<RGResourceMode::Vertex>;
    using RGBufferIndexId = RGBufferModeId<RGResourceMode::Index>;
    using RGBufferConstantId = RGBufferModeId<RGResourceMode::Constant>;

    //------------------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------------------//

    struct RenderGraphResourceDescriptorId
    {
        constexpr static uint64_t invalid_id = uint64_t(-1);

        RenderGraphResourceDescriptorId() : id(invalid_id) {}
        RenderGraphResourceDescriptorId(uint64_t viewId, RenderGraphResourceId resourceHandle)
            : id(invalid_id)
        {
            const uint32_t _resourceId = resourceHandle.id;
            id = (viewId << 32) | _resourceId;
        }

        uint64_t GetViewId() const { return (id >> 32); }
        uint64_t GetResourceId() const
        {
            return (uint64_t)static_cast<uint32_t>(id);
        }

        RenderGraphResourceId operator*() const
        {
            return RenderGraphResourceId(GetResourceId());
        }

        void Invalidate() { id = invalid_id; }
        bool IsValid() const { return id != invalid_id; }

        bool operator == (RenderGraphResourceDescriptorId const& other) const { return this->id == other.id; }
        bool operator != (RenderGraphResourceDescriptorId const& other) const { return this->id != other.id; }

        uint64_t id;
    };
    
    enum class RGDescriptorType : uint8_t
    {
        ReadOnly,
        ReadWrite,
        RenderTarget,
        DepthStencil
    };

    enum RGTextureDescriptorFlags : uint8_t
    {
        RGTextureDescriptorFlag_None = 0x0,
        RGTextureDescriptorFlag_DepthReadOnly = 0x1
    };

    enum RGTextureChannelMapping : uint32_t
    {
        RGTextureChannelMapping_Red,
        RGTextureChannelMapping_Green,
        RGTextureChannelMapping_Blue,
        RGTextureChannelMapping_Alpha,
        RGTextureChannelMapping_Zero,
        RGTextureChannelMapping_One,
    };

    constexpr RGTextureChannelMapping CustomTextureChannelMapping(RGTextureChannelMapping R, RGTextureChannelMapping G,
        RGTextureChannelMapping B, RGTextureChannelMapping A)
    {
        constexpr uint32_t TextureChannelMappingMask = 0x7;
        constexpr uint32_t TextureChannelMappingShift = 3;
        constexpr uint32_t TextureChannelMappingAlwaysSetBit = 1 << (TextureChannelMappingShift * 4);

        return RGTextureChannelMapping(
            (((R)&TextureChannelMappingMask) << (TextureChannelMappingShift * 0)) |
            (((G)&TextureChannelMappingMask) << (TextureChannelMappingShift * 1)) |
            (((B)&TextureChannelMappingMask) << (TextureChannelMappingShift * 2)) |
            (((A)&TextureChannelMappingMask) << (TextureChannelMappingShift * 3)) |
            TextureChannelMappingAlwaysSetBit);
    }

    constexpr RGTextureChannelMapping RGDefaultTextureChannelMapping = CustomTextureChannelMapping(RGTextureChannelMapping_Red, RGTextureChannelMapping_Green,
        RGTextureChannelMapping_Blue, RGTextureChannelMapping_Alpha);
    constexpr RGTextureChannelMapping RGAlphaOneTextureChannelMapping = CustomTextureChannelMapping(RGTextureChannelMapping_Red, RGTextureChannelMapping_Green,
        RGTextureChannelMapping_Blue, RGTextureChannelMapping_One);
    
    struct RGTextureDescriptorDesc
    {
        uint32_t firstSlice = 0;
        uint32_t sliceCount = static_cast<uint32_t>(-1);
        uint32_t firstMip = 0;
        uint32_t mipCount = static_cast<uint32_t>(-1);

        RGTextureDescriptorFlags flags = RGTextureDescriptorFlag_None;
        RGTextureChannelMapping channelMapping = RGDefaultTextureChannelMapping;
        // std::strong_ordering operator<=>(RGTextureDescriptorDesc const& other) const = default;
        //TODO:
        // bool operator == (RGTextureDescriptorDesc const& other) const = default;
        // bool operator != (RGTextureDescriptorDesc const& other) const { return this->id != other.id; }
    };

    struct RGBufferDescriptorDesc
    {
        uint64_t offset = 0;
        uint64_t size = uint64_t(-1);
        // std::strong_ordering operator<=>(RGBufferDescriptorDesc const& other) const = default;
    };
    struct RGTextureInitialData
    {
        void const* data;
        uint64_t rowPitch;
        uint64_t slicePitch;
    };

    template<RGResourceType ResourceType, RGDescriptorType ResourceViewType>
    struct TypedRenderGraphResourceDescriptorId : RenderGraphResourceDescriptorId
    {
        using RenderGraphResourceDescriptorId::RenderGraphResourceDescriptorId;
        using RenderGraphResourceDescriptorId::operator*;

        auto GetResourceId() const
        {
            if(ResourceType == RGResourceType::Buffer)
            {
                return RGBufferId(RenderGraphResourceDescriptorId::GetResourceId());
            }else if(ResourceType == RGResourceType::Texture)
            {
                return RGTextureId(RenderGraphResourceDescriptorId::GetResourceId());
            }
        }

        auto operator*() const
        {
            if(ResourceType == RGResourceType::Buffer)
            {
                return RGBufferId(GetResourceId());
            }else if(ResourceType == RGResourceType::Texture)
            {
                return RGTextureId(GetResourceId());
            }
        }
    };

    using RGRenderTargetId      =   TypedRenderGraphResourceDescriptorId<RGResourceType::Texture, RGDescriptorType::RenderTarget>;
    using RGDepthStencilId      =   TypedRenderGraphResourceDescriptorId<RGResourceType::Texture, RGDescriptorType::DepthStencil>;
    using RGTextureReadOnlyId   =   TypedRenderGraphResourceDescriptorId<RGResourceType::Texture, RGDescriptorType::ReadOnly>;
    using RGTextureReadWriteId  =   TypedRenderGraphResourceDescriptorId<RGResourceType::Texture, RGDescriptorType::ReadWrite>;

    using RGBufferReadOnlyId    =   TypedRenderGraphResourceDescriptorId<RGResourceType::Buffer, RGDescriptorType::ReadOnly>;
    using RGBufferReadWriteId   =   TypedRenderGraphResourceDescriptorId<RGResourceType::Buffer, RGDescriptorType::ReadWrite>;

    //------------------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------------------//

    struct RGAllocationId : RGResourceId
    {
        using RGResourceId::RGResourceId;
    };
  
}

namespace std
{
    template <> struct hash<EveryRay_Core::RGTextureId>
    {
        uint64_t operator()(EveryRay_Core::RGTextureId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGBufferId>
    {
        uint64_t operator()(EveryRay_Core::RGBufferId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGTextureReadOnlyId>
    {
        uint64_t operator()(EveryRay_Core::RGTextureReadOnlyId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGTextureReadWriteId>
    {
        uint64_t operator()(EveryRay_Core::RGTextureReadWriteId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGRenderTargetId>
    {
        uint64_t operator()(EveryRay_Core::RGRenderTargetId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGDepthStencilId>
    {
        uint64_t operator()(EveryRay_Core::RGDepthStencilId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGBufferReadOnlyId>
    {
        uint64_t operator()(EveryRay_Core::RGBufferReadOnlyId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };

    template <> struct hash<EveryRay_Core::RGBufferReadWriteId>
    {
        uint64_t operator()(EveryRay_Core::RGBufferReadWriteId const& h) const
        {
            return hash<decltype(h.id)>()(h.id);
        }
    };
}