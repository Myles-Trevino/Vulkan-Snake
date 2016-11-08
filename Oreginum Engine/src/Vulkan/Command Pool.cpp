#include "../Oreginum/Core.hpp"
#include "Command Pool.hpp"

Oreginum::Vulkan::Command_Pool::Command_Pool(const Device& device,
	 uint32_t queue_family_index, vk::CommandPoolCreateFlags flags) : device(device)
{
	vk::CommandPoolCreateInfo pool_information{flags, queue_family_index};

	if(device.get().createCommandPool(&pool_information, nullptr, &command_pool) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan command pool.");
}

Oreginum::Vulkan::Command_Pool::~Command_Pool(){ device.get().destroyCommandPool(command_pool); }