#pragma once

#include <iostream>
#include <chrono>

class ProfileTimer
{
public:
	ProfileTimer(const char* message) :message(message)
	{
		startTime = std::chrono::high_resolution_clock::now();
	}
	~ProfileTimer()
	{
		Stop();
	}
	void Stop()
	{
		auto endTimePoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();
		auto duration = end - start;
		double ms = duration * 0.001;

		std::cout << "Timer: " << message << ms << "ms" << std::endl;
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	const char* message;
};