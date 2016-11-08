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
			vk::FenceCreateFlags flags = vk::FenceCreateFlagBits::eSignaled);

		const vk::Fence& get() const { return fence; }

	private:
		const Device *device;

		vk::Fence fence;

		void destroy();
	};
}