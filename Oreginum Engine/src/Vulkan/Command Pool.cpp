#include "../Oreginum/Core.hpp"
#include "Command Pool.hpp"

void Oreginum::Vulkan::Command_Pool::destroy()
{ if(command_pool) vkDestroyCommandPool(device->get(), command_pool, nullptr); }

void Oreginum::Vulkan::Command_Pool::initialize(const Device *device,
	VkCommandPoolCreateFlags flags, uint32_t queue_family_index)
{
	this->device = device;
	destroy();

	VkCommandPoolCreateInfo pool_information;
	pool_information.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_information.pNext = nullptr;
	pool_information.flags = flags;
	pool_information.queueFamilyIndex = queue_family_index;

	if(vkCreateCommandPool(device->get(), &pool_information, nullptr, &command_pool) !=
		VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan command pool.");
}