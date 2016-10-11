#pragma once
#include <string>
#include <chrono>

namespace Oreginum
{
	namespace Core
	{
		void error(const std::string& error);
		int refresh_rate();
		double time();
	}
}
