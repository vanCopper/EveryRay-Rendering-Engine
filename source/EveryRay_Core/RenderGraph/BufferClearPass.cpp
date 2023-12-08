#include "BufferClearPass.h"

#include "BufferResource.h"

namespace EveryRay_Core
{
    BufferClearPass::BufferClearPass(std::string name) noexcept
        :
        ER_Pass(std::move(name))
    {
    }

    void BufferClearPass::Execute(RenderGraphContext& rgContext) const
    {
        bufferResource->Clear(rgContext);
    }
}
