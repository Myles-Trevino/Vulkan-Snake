#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Semaphore
	{
	public:
		Semaphore(const Device& device);
		~Semaphore();

		const vk::Semaphore& get() const { return semaphore; }

	private:
		const Device& device;

		vk::Semaphore semaphore;
	};
}