#pragma once
#include <memory>

#include "../RHI/ER_RHI.h"

namespace EveryRay_Core
{
    class RenderGraphResourcePool
    {
        struct PooledTexture
        {
            PooledTexture(const PooledTexture& other) = delete; // for std::vector
            std::unique_ptr<ER_RHI_GPUTexture> texture;
            uint64_t last_used_frame = 0;
        };

        struct PooledBuffer
        {
            PooledBuffer(const PooledBuffer& other) = delete; // for std::vector
            std::unique_ptr<ER_RHI_GPUBuffer> buffer;
            uint64_t last_used_frame = 0;
        };
        
    public:
        explicit RenderGraphResourcePool(ER_RHI* rhi) : rhi(rhi) {}

        void Tick()
        {
            // for (size_t i = 0; i < texture_pool.size();)
            // {
            //     PooledTexture& resource = texture_pool[i].first;
            //     bool active = texture_pool[i].second;
            //     if (!active && resource.last_used_frame + 4 < frame_index)
            //     {
            //         std::swap(texture_pool[i], texture_pool.back());
            //         texture_pool.pop_back();
            //     }
            //     else ++i;
            // }
            ++frame_index;
        }

        ER_RHI_GPUTexture* AllocateTexture(/*ER_RHI_TextureDesc const& desc*/)
        {
            // for (auto& [pool_texture, active] : texture_pool)
            // {
            //     if (!active && pool_texture.texture->GetDesc().IsCompatible(desc))
            //     {
            //         pool_texture.last_used_frame = frame_index;
            //         active = true;
            //         return pool_texture.texture.get();
            //     }
            // }
            // auto& texture = texture_pool.emplace_back(std::pair{ PooledTexture{ std::make_unique<ER_RHI_GPUTexture>(device, desc), frame_index}, true }).first.texture;
            // return texture.get();
            return nullptr;
        }
        
        void ReleaseTexture(ER_RHI_GPUTexture* texture)
        {
            // for (auto& [pooled_texture, active] : texture_pool)
            // {
            //     auto& texture_ptr = pooled_texture.texture;
            //     if (active && texture_ptr.get() == texture)
            //     {
            //         active = false;
            //         break;
            //     }
            // }
        }

        ER_RHI_GPUBuffer* AllocateBuffer(/*ER_RHI_BufferDesc const& desc*/)
        {
            // for (auto& [pool_buffer, active] : buffer_pool)
            // {
            //     if (!active && pool_buffer.buffer->GetDesc().IsCompatible(desc))
            //     {
            //         pool_buffer.last_used_frame = frame_index;
            //         active = true;
            //         return pool_buffer.buffer.get();
            //     }
            // }
            // auto& buffer = buffer_pool.emplace_back(std::pair{ PooledBuffer{ std::make_unique<ER_RHI_GPUBuffer>(device, desc), frame_index}, true }).first.buffer;
            // return buffer.get();
            return nullptr;
        }

        void ReleaseBuffer(/*GfxBuffer* buffer*/)
        {
            // for (auto& [pooled_buffer, active] : buffer_pool)
            // {
            //     auto& buffer_ptr = pooled_buffer.buffer;
            //     if (active && buffer_ptr.get() == buffer)
            //     {
            //         active = false;
            //     }
            // }
        }

        ER_RHI* GetRHI() const { return rhi; }

    private:
        ER_RHI* rhi;
        uint64_t frame_index = 0;
        // std::vector<std::pair<PooledTexture, bool>> texture_pool;
        // std::vector<std::pair<PooledBuffer, bool>> buffer_pool;
    };
    using RGResourcePool = RenderGraphResourcePool;
}
