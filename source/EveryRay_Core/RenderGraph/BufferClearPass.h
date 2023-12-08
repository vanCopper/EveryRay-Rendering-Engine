#pragma once
#include <memory>

#include "ER_Pass.h"

namespace EveryRay_Core
{
    class BufferResource;
}

namespace EveryRay_Core
{
    class BufferClearPass : public ER_Pass
    {
    public:
        BufferClearPass(std::string name) noexcept;

        virtual void Execute(RenderGraphContext& rgContext) const override;

    private:
        std::shared_ptr<BufferResource> bufferResource;
    };
}
