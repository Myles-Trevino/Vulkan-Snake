#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Semaphore
	{
	public:
		Semaphore(){}
		Semaphore(const Device& device);
		Semaphore *Semaphore::operator=(Semaphore other){ swap(&other); return this; }
		~Semaphore();

		const vk::Semaphore& get() const { return *semaphore; }

	private:
		const Device *device;
		std::shared_ptr<vk::Semaphore> semaphore = std::make_shared<vk::Semaphore>();

		void swap(Semaphore *other);
	};
}