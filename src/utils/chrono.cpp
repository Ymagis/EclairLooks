#include "chrono.h"

using namespace std::chrono;

Chrono::Chrono()
	: _paused(false)
{

}

Chrono::~Chrono()
{

}

void Chrono::start()
{
	// reset chrono if already running
	_startTime = ClockT::now();
	_current_duration = DurationT::zero();
}

void Chrono::pause()
{
	if(!_paused)
	{
		_current_duration = ClockT::now() - _startTime;
		_paused = true;
	}
}

void Chrono::resume()
{
	if(_paused == true)
	{
		_startTime = ClockT::now();
		_paused = false;
	}
}

float Chrono::ellapsed(DurationType dt)
{
	if(!_paused)
	{
		_current_duration += ClockT::now() - _startTime;
		_startTime = ClockT::now();
	}

	float ellapsed = 0;

	switch(dt)
	{
		case MINUTES :
			ellapsed = duration_cast<minutes>(_current_duration).count();
			break;
		case SECONDS :
			ellapsed = duration_cast<seconds>(_current_duration).count();
			break;
		case MILLISECONDS :
			ellapsed = duration_cast<milliseconds>(_current_duration).count();
			break;
		case NANOSECONDS :
			ellapsed = duration_cast<nanoseconds>(_current_duration).count();
	}

	return ellapsed;
}