#include <windows.h>
#include "Error.hpp"

void error(const std::string& error)
{
	MessageBox(NULL, error.c_str(), "Oreginum Engine", MB_ICONERROR);
	std::exit(EXIT_FAILURE);
}