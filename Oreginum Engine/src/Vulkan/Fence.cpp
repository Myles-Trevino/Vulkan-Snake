#include "../Oreginum/Core.hpp"
#include "Fence.hpp"

void Oreginum::Vulkan::Fence::destroy(){ if(fence) device->get().destroyFence(fence); }

void Oreginum::Vulkan::Fence::initialize(const Device *device, vk::FenceCreateFlags flags)
{
	this->device = device;
	destroy();

	vk::FenceCreateInfo fence_information;
	fence_information.setFlags(flags);

	if(device->get().createFence(&fence_information, nullptr, &fence) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan fence.");
}