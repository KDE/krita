#pragma once
#include <chrono>
#include <utility>

template<typename TickType = std::chrono::nanoseconds>
struct Benchmark
{
	template<typename F, typename ...Args>
	static TickType Run(F func, Args&&... args)
	{
		auto StartPoint = std::chrono::system_clock::now();

		func(std::forward<Args>(args)...);

		auto Duration = std::chrono::duration_cast<TickType>(std::chrono::system_clock::now() - StartPoint);

		return Duration;
	}
};
