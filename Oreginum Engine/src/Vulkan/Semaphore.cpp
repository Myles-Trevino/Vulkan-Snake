#include "../Oreginum/Core.hpp"
#include "Semaphore.hpp"

Oreginum::Vulkan::Semaphore::Semaphore(const Device& device) : device(device)
{
	vk::SemaphoreCreateInfo semaphore_information;

	if(device.get().createSemaphore(&semaphore_information, nullptr, &semaphore) != 
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan semaphore.");
}

Oreginum::Vulkan::Semaphore::~Semaphore(){ device.get().destroySemaphore(semaphore); }