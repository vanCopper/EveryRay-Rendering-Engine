#pragma once

#include "../Common.h"
#include "../ThirdParty/renderdoc_app.h"

namespace EveryRay_Core
{
    class ER_RenderDocCapture
    {
    public:
        static void Initialize();
        
        static bool IsAvailable();
        static bool IsFrameCapturing();
        static bool IsTargetControlConnected();

        static void ApiVersion(int *major, int *minor, int *patch);

        static void TriggerCapture();
        static void TriggerMultiFrameCapture(uint32_t numFrames);
        static void EndFrameCapture();

        static uint32_t LaunchReplayUI(uint32_t connectTargetControl = 1, const char *cmdLine = nullptr);

    private:
        static RENDERDOC_API_1_1_1* mRenderDocAPI;
    };

}
