#pragma once

#include <chrono>

class Chrono
{
public:
	Chrono();
	~Chrono();

public:
	void start();
	void pause();
	void resume();

	enum DurationType { MINUTES, SECONDS, MILLISECONDS, NANOSECONDS };
	float ellapsed(DurationType dt = MILLISECONDS);

private:
	typedef std::chrono::duration<double, std::nano> DurationT;
	typedef std::chrono::high_resolution_clock ClockT;
	typedef std::chrono::time_point<ClockT> TimePointT;

	DurationT _current_duration;
	TimePointT _startTime;

	bool _paused;
};