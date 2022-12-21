#pragma once

#include <chrono>
using namespace std::chrono; // TODO: move to .cpp so namespace doesn't spread

class Timer
{
public:
	Timer()
	{
		Start();
		Stop();
	}

	void Start()
	{
		startTimeMs = duration_cast<milliseconds>( system_clock::now().time_since_epoch() );
		endTimeMs = startTimeMs;
	}

	void Stop()
	{
		endTimeMs = duration_cast<milliseconds>( system_clock::now().time_since_epoch() );
	}

	uint64_t GetElapsed()
	{
		return ( endTimeMs - startTimeMs ).count();
	}

private:
	milliseconds startTimeMs;
	milliseconds endTimeMs;
};