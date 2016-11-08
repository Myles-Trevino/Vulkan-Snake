#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Surface.hpp"

Oreginum::Vulkan::Surface::Surface(const Instance& instance) : instance(instance)
{
	vk::Win32SurfaceCreateInfoKHR surface_information
	{{}, Oreginum::Window::get_instance(), Oreginum::Window::get()};

	if(instance.get().createWin32SurfaceKHR(&surface_information, nullptr, &surface) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan surface.");
}

Oreginum::Vulkan::Surface::~Surface(){ instance.get().destroySurfaceKHR(surface); }