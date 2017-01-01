#include "../Oreginum/Core.hpp"
#include "Fence.hpp"

Oreginum::Vulkan::Fence::Fence(const Device& device, vk::FenceCreateFlags flags)
	: device(&device)
{
	vk::FenceCreateInfo fence_information{flags};
	if(device.get().createFence(&fence_information, nullptr, fence.get()) != 
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan fence.");
}

Oreginum::Vulkan::Fence::~Fence()
{ if(fence.unique() && *fence) device->get().destroyFence(*fence); };

void Oreginum::Vulkan::Fence::swap(Fence *other)
{
	std::swap(device, other->device);
	std::swap(fence, other->fence);
}