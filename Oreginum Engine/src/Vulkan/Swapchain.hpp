#pragma once
#include "Instance.hpp"
#include "Device.hpp"
#include "Image.hpp"
#include "Surface.hpp"

namespace Oreginum::Vulkan
{
	class Swapchain
	{
	public:
		Swapchain(const Instance& instance, const Surface& surface,
			Device *device, const Command_Buffer& command_buffer);
		~Swapchain(){ device->get().destroySwapchainKHR(swapchain); }

		const vk::SwapchainKHR& get() const { return swapchain; }
		const std::vector<Image>& get_images() const { return images; }
		const vk::Extent2D& get_extent() const { return extent; }

	private:
		Device *device;
		const Instance& instance;

		vk::Extent2D extent;
		vk::SwapchainKHR swapchain;
		std::vector<Image> images;
	};
}