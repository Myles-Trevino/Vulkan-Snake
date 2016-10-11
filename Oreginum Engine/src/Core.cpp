#include <windows.h>
#include "Core.hpp"

namespace
{
	double time_since_epoch(){ return (double)std::chrono::high_resolution_clock::
		now().time_since_epoch().count()/1000000000; }
	static double initial_time{time_since_epoch()};
};

void Oreginum::Core::error(const std::string& error)
{
	MessageBox(NULL, error.c_str(), "Oreginum Engine", MB_ICONERROR);
	std::exit(EXIT_FAILURE);
}

int Oreginum::Core::refresh_rate()
{
	DEVMODE devmode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	return devmode.dmDisplayFrequency;
}

double Oreginum::Core::time(){ return time_since_epoch()-initial_time; }