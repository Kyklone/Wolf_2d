#pragma once

#include <chrono>

class frame_t
{
public:

	float	frame_timer()
	{
		auto tp1 = std::chrono::system_clock::now();
		auto tp2 = std::chrono::system_clock::now();

		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		return (elapsedTime.count());
	}
};

