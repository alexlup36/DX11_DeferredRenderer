#ifndef TIMER_H
#define TIMER_H

// ----------------------------------------------------------------------------
// Includes

// ----------------------------------------------------------------------------

class Timer
{
public:

	// ------------------------------------------------------------------------
	// Constructor destructor

	Timer();
	~Timer();

	// ------------------------------------------------------------------------
	// Public methods

	void StartTimer();
	double GetTime();
	double GetFrameTime();
	
	// Inline
	inline void IncrementFrameCount() { m_iFrameCount++; }
	inline void ResetFrameCount() { m_iFrameCount = 0; }
	inline int GetFrameCount() { return m_iFrameCount; }

	// ------------------------------------------------------------------------

private:

	// ------------------------------------------------------------------------
	// Private members

	double m_dCountsPerSecond = 0.0f;
	__int64 m_CounterStart = 0;

	int m_iFrameCount = 0;
	int m_iFramesPerSecond = 0;

	__int64 m_FrameTimeOld = 0;
	double m_dFrameTime;

	// ------------------------------------------------------------------------
};

#endif // TIMER_H