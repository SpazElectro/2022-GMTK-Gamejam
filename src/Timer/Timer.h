#pragma once

#include <chrono>

#define ELAPSE_TIME 1000
#define FPS_ELAPSE_TIME 1000

class Timer
{
public:
	/*
	Timer handles time counting.
	*/
	Timer();

	/*
	Function gets the time that has left sence the beginning of the counting.
	Output:
		the time that has left.
	*/
	unsigned long long ElapsedMillisecs() const;

	std::chrono::steady_clock::time_point startTime;
};