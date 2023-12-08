#include "GraphicsResource.h"

namespace EveryRay_Core
{
    ID3D11DeviceContext* GraphicsResource::GetContext(ER_RHI_DX11& rhi) noexcept
    {
        return rhi.GetContext();
    }

    ID3D11Device* GraphicsResource::GetDevice(ER_RHI_DX11& rhi) noexcept
    {
        return rhi.GetDevice();
    }
}
