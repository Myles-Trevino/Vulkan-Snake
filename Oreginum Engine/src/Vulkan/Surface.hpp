#pragma once
#include "Instance.hpp"

namespace Oreginum::Vulkan
{
	class Surface
	{
	public:
		Surface(const Instance& instance);
		~Surface();

		vk::SurfaceKHR get() const { return surface; }

	private:
		const Instance& instance;

		vk::SurfaceKHR surface;
	};
}