#include "ER_Pass.h"

namespace EveryRay_Core
{
    ER_Pass::ER_Pass(std::string name) noexcept
        :
        name(std::move(name))
    {
    }

    void ER_Pass::Execute(RenderGraphContext& rgContext) const
    {
    }

    void ER_Pass::Reset()
    {
    }

    const std::string& ER_Pass::GetName() const noexcept
    {
        return name;
    }

    void ER_Pass::Finalize()
    {
    }

    ER_Pass::~ER_Pass()
    {}
}
