#include "ER_CPUProfiler.h"
#include "ER_Utility.h"

namespace Library {
	
	ER_CPUProfiler::ER_CPUProfiler()
	{

	}

	ER_CPUProfiler::~ER_CPUProfiler()
	{

	}

	void ER_CPUProfiler::BeginCPUTime(const std::string& aEventName, bool toLog /*= true*/)
	{
		auto startTimer = std::chrono::high_resolution_clock::now();
		auto it = mEventsCPUTime.find(aEventName);
		if (it == mEventsCPUTime.end())
			mEventsCPUTime.emplace(aEventName, startTimer);
		else
			it->second = startTimer;
	}

	void ER_CPUProfiler::EndCPUTime(const std::string& aEventName)
	{
		auto endTimer = std::chrono::high_resolution_clock::now();
		auto it = mEventsCPUTime.find(aEventName);
		if (it == mEventsCPUTime.end())
			return;
		else
		{
			std::chrono::duration<double> finalTime = endTimer - it->second;
			std::string message = "[CPU Profiler] CPU time of <" + aEventName + "> is " + std::to_string(finalTime.count()) + "s\n";
			OutputDebugString(ER_Utility::ToWideString(message).c_str());
		}
	}

}