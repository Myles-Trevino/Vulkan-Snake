#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Fence
	{
	public:
		Fence(const Device& device, vk::FenceCreateFlags flags =
			vk::FenceCreateFlagBits::eSignaled);
		Fence *Fence::operator=(Fence other){ swap(&other); return this; }
		~Fence();

		const vk::Fence& get() const { return *fence; }

	private:
		const Device *device;
		std::shared_ptr<vk::Fence> fence = std::make_shared<vk::Fence>();

		void swap(Fence *other);
	};
}