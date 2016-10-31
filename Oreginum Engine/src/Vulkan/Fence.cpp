#include "../Oreginum/Core.hpp"
#include "Fence.hpp"

void Oreginum::Vulkan::Fence::destroy()
{ if(fence) vkDestroyFence(device->get(), fence, nullptr); }

void Oreginum::Vulkan::Fence::initialize(const Device *device, VkFenceCreateFlags flags)
{
	this->device = device;
	destroy();

	VkFenceCreateInfo fence_information;
	fence_information.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_information.pNext = nullptr;
	fence_information.flags = flags;

	if(vkCreateFence(device->get(), &fence_information, nullptr, &fence) != VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan fence.");
}