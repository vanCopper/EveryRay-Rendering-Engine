#pragma once

#include <d3d12.h>

#include "../RHI/DX12/ER_RHI_DX12.h"
#include "RenderGraphPass.h"

namespace EveryRay_Core
{
    struct RGTextureDesc
    {
        RGTextureType type = RGTextureType_2D;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;
        uint32_t array_size = 1;
        uint32_t mip_levels = 1;
        uint32_t sample_count = 1;
        D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_DEFAULT;
        ER_RHI_RESOURCE_MISC_FLAG misc_flags = ER_RESOURCE_MISC_NONE;
        D3D12_CLEAR_VALUE clear_value{};
        ER_RHI_FORMAT format = ER_FORMAT_UNKNOWN;
    };

    struct RGBufferDesc
    {
        uint64_t size = 0;
        uint32_t stride = 0;
        D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_DEFAULT;
        ER_RHI_RESOURCE_MISC_FLAG misc_flags = ER_RESOURCE_MISC_NONE;
        ER_RHI_FORMAT format = ER_FORMAT_UNKNOWN;
    };
    
    class RenderGraphBuilder
    {
    public:
    
    };
}
