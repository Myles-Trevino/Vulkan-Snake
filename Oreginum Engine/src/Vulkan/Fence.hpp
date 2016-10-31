#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Fence
	{
	public:
		Fence(){};
		~Fence(){ destroy(); };

		void initialize(const Device *device,
			VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);

		VkFence get() const { return fence; }

	private:
		const Device *device;

		VkFence fence;

		void destroy();
	};
}