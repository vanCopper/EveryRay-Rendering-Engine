#include "ER_RenderDocCapture.h"
#include "../ER_Utility.h"
#include "../ThirdParty/spdlog/spdlog.h"

namespace EveryRay_Core
{
    RENDERDOC_API_1_1_1* ER_RenderDocCapture::mRenderDocAPI = nullptr;

    void ER_RenderDocCapture::Initialize()
    {
        HMODULE renderDocDLL = GetModuleHandleA("renderdoc.dll");

        if(renderDocDLL == nullptr)
        {
            std::string renderdocPath = ER_Utility::GetFilePath("external\\Renderdoc\\lib\\renderdoc.dll");
            renderDocDLL = LoadLibraryA(renderdocPath.c_str());
        }

        if(renderDocDLL == nullptr)
        {
            return;
        }

        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(renderDocDLL, "RENDERDOC_GetAPI");
        if(RENDERDOC_GetAPI == nullptr)
        {
            return;
        }

        const int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_1, (void**)&mRenderDocAPI);
        if(ret != 1)
        {
            return;
        }

        int major, minor, patch;
        ApiVersion(&major, &minor, &patch);
        spdlog::info("RenderDoc API: {}.{}.{}", major, minor, patch);

        mRenderDocAPI->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, 1);
        mRenderDocAPI->SetCaptureOptionU32(eRENDERDOC_Option_RefAllResources, 1);
        mRenderDocAPI->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1);

    }

    bool ER_RenderDocCapture::IsTargetControlConnected()
    {
        if(mRenderDocAPI)
        {
            return mRenderDocAPI->IsTargetControlConnected() == 1;
        }
        return false;
    }

    bool ER_RenderDocCapture::IsAvailable()
    {
        return (mRenderDocAPI != nullptr);
    }

    bool ER_RenderDocCapture::IsFrameCapturing()
    {
        if(mRenderDocAPI)
        {
            return mRenderDocAPI->IsFrameCapturing() == 1;
        }
        return false;
    }

    void ER_RenderDocCapture::ApiVersion(int* major, int* minor, int* patch)
    {
        if(mRenderDocAPI)
        {
            mRenderDocAPI->GetAPIVersion(major, minor, patch);
        }
    }

    void ER_RenderDocCapture::TriggerCapture()
    {
        if(mRenderDocAPI)
        {
            mRenderDocAPI->TriggerCapture();
        }
    }

    void ER_RenderDocCapture::TriggerMultiFrameCapture(uint32_t numFrames)
    {
        if(mRenderDocAPI)
        {
            mRenderDocAPI->TriggerMultiFrameCapture(numFrames);
        }
    }

    void ER_RenderDocCapture::EndFrameCapture()
    {
        if(mRenderDocAPI)
        {
            mRenderDocAPI->EndFrameCapture(nullptr, nullptr);
        }
    }

    uint32_t ER_RenderDocCapture::LaunchReplayUI(uint32_t connectTargetControl, const char* cmdLine)
    {
        if(mRenderDocAPI)
        {
            return mRenderDocAPI->LaunchReplayUI(connectTargetControl, cmdLine);
        }

        return 0;
    }
}
