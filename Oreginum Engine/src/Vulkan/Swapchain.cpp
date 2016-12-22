#include <algorithm>
#include <array>
#include "../Oreginum/Core.hpp"
#include "Swapchain.hpp"
#include "../Oreginum/Window.hpp"

void Oreginum::Vulkan::Swapchain::initialize(const Instance& instance,
	const Surface& surface, Device *device) 
{
	//Create swapchain
	device->update();
	extent = device->get_surface_capabilities().currentExtent;

	vk::SwapchainKHR old_swapchain{*swapchain};
	vk::SwapchainCreateInfoKHR swapchain_information{{}, surface.get(), std::max(
		MINIMUM_IMAGE_COUNT, device->get_surface_capabilities().minImageCount),
		FORMAT.format, FORMAT.colorSpace, extent, 1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive, 0, nullptr,
		device->get_surface_capabilities().currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eMailbox,
		VK_TRUE, old_swapchain};

	std::array<uint32_t, 2> queue_indices{device->get_graphics_queue_family_index(),
		device->get_present_queue_family_index()};
	if(queue_indices[0] == queue_indices[1])
	{
		swapchain_information.setImageSharingMode(vk::SharingMode::eConcurrent);
		swapchain_information.setQueueFamilyIndexCount(2);
		swapchain_information.setPQueueFamilyIndices(queue_indices.data());
	}

	if(device->get().createSwapchainKHR(&swapchain_information, nullptr, swapchain.get()) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create Vulkan swapchain.");
	if(old_swapchain) device->get().destroySwapchainKHR(old_swapchain);

	//Create image views
	images.clear();
	std::vector<vk::Image> image_handles{device->get().getSwapchainImagesKHR(*swapchain).value};
	for(auto i : image_handles) images.push_back({*device, i, FORMAT.format});
}

Oreginum::Vulkan::Swapchain::~Swapchain()
{ if(swapchain.unique() && *swapchain) device->get().destroySwapchainKHR(*swapchain); }

void Oreginum::Vulkan::Swapchain::swap(Swapchain *other)
{
	std::swap(this->device, other->device);
	std::swap(this->surface, other->surface);
	std::swap(this->instance, other->instance);
	std::swap(this->extent, other->extent);
	std::swap(this->swapchain, other->swapchain);
	std::swap(this->images, other->images);
}