#include <algorithm>
#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Swapchain.hpp"
#include "../Oreginum/Window.hpp"

Oreginum::Vulkan::Swapchain::Swapchain(const Instance& instance,
	const Surface& surface, Device *device, const Command_Buffer& command_buffer)
	: instance(instance), device(device)
{
	//Create swapchain
	device->update();
	extent = device->get_surface_capabilities().currentExtent;

	vk::SwapchainKHR old_swapchain{swapchain};
	vk::SwapchainCreateInfoKHR swapchain_information{{}, surface.get(), std::max(
		Core::SWAPCHAIN_MINIMUM_IMAGE_COUNT, device->get_surface_capabilities().minImageCount),
		Core::SWAPCHAIN_FORMAT.format, Core::SWAPCHAIN_FORMAT.colorSpace, extent, 1,
		vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0, nullptr,
		device->get_surface_capabilities().currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eMailbox,
		VK_TRUE, old_swapchain};

	if(device->get().createSwapchainKHR(&swapchain_information, nullptr, &swapchain) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create Vulkan swapchain.");
	if(old_swapchain) device->get().destroySwapchainKHR(old_swapchain);

	//Create image views
	std::vector<vk::Image> image_handles{device->get().getSwapchainImagesKHR(swapchain).value};
	
	for(size_t i{}; i < images.size(); ++i) images.push_back({*device,
		command_buffer, image_handles[i], Core::SWAPCHAIN_FORMAT.format});
}