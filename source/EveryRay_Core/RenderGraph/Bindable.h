#pragma once
#include "RenderGraphContext.h"

namespace EveryRay_Core
{
    class Bindable
    {
    public:
        virtual void Bind(RenderGraphContext& context) = 0;
        virtual ~Bindable() = default;
    };
}
