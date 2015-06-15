#include "Timer.h"

// ----------------------------------------------------------------------------
// Includes
#include <windows.h>

// ----------------------------------------------------------------------------

Timer::Timer()
{
}

// ----------------------------------------------------------------------------

Timer::~Timer()
{
}

// ----------------------------------------------------------------------------

void Timer::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	m_dCountsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	m_CounterStart = frequencyCount.QuadPart;
}

// ----------------------------------------------------------------------------

double Timer::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	return (double)((currentTime.QuadPart - m_CounterStart) / m_dCountsPerSecond);
}

// ----------------------------------------------------------------------------

double Timer::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - m_FrameTimeOld;
	m_FrameTimeOld = currentTime.QuadPart;

	if (tickCount < 0)
	{
		tickCount = 0;
	}

	return (float)(tickCount / m_dCountsPerSecond);
}

// ----------------------------------------------------------------------------