#pragma once
#include "../RHI/DX11/ER_RHI_DX11.h"

namespace EveryRay_Core
{
    class GraphicsResource
    {
    public:
        static ID3D11DeviceContext* GetContext(ER_RHI_DX11& rhi) noexcept;
        static ID3D11Device* GetDevice(ER_RHI_DX11& rhi) noexcept;
    };
}
