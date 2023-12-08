#pragma once

namespace EveryRay_Core
{
    class RenderGraphContext;
}

namespace EveryRay_Core
{
    class BufferResource
    {
    public:
        virtual ~BufferResource() = default;
        virtual void BindAsBuffer(RenderGraphContext& ) = 0;
        virtual void BindAsBuffer(RenderGraphContext&, BufferResource*) = 0;
        virtual void Clear(RenderGraphContext& ) = 0;
    };
}
