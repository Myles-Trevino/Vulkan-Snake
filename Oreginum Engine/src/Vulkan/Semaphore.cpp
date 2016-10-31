#include "../Oreginum/Core.hpp"
#include "Semaphore.hpp"

void Oreginum::Vulkan::Semaphore::destroy()
{ if(semaphore) vkDestroySemaphore(device->get(), semaphore, nullptr); }

void Oreginum::Vulkan::Semaphore::initialize(const Device *device)
{
	this->device = device;
	destroy();

	VkSemaphoreCreateInfo semaphore_information;
	semaphore_information.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_information.pNext = nullptr;
	semaphore_information.flags = NULL;

	if(vkCreateSemaphore(device->get(), &semaphore_information, nullptr, &semaphore) != 
		VK_SUCCESS) Oreginum::Core::error("Could not create Vulkan semaphores.");
}