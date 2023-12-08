#pragma once
#include <string>

namespace EveryRay_Core
{
    class RenderGraphContext;
}

namespace EveryRay_Core
{
    class ER_Pass
    {
    public:
        ER_Pass(std::string name) noexcept;
        
        virtual void Execute(RenderGraphContext& rgContext) const;
        virtual void Reset();
        const std::string& GetName() const noexcept;
        virtual void Finalize();
        
        virtual ~ER_Pass();

    private:
        std::string name;
    };
}
