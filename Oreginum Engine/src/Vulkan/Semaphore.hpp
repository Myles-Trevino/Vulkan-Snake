#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Semaphore
	{
	public:
		Semaphore(){};
		~Semaphore(){ destroy(); }

		void initialize(const Device *device);

		VkSemaphore get() const { return semaphore; }

	private:
		const Device *device;

		VkSemaphore semaphore;

		void destroy();
	};
}