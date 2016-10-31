#pragma once
#include "Instance.hpp"
#include "Device.hpp"
#include "Image View.hpp"

namespace Oreginum::Vulkan
{
	class Swapchain
	{
	public:
		Swapchain(){}
		~Swapchain();

		void initialize(const Instance *instance, Device *device);

		VkSwapchainKHR get() const { return swapchain; }
		const std::vector<Image_View>& get_image_views() const { return image_views; }
		const VkExtent2D& get_extent() const { return extent; }

	private:
		Device *device;

		VkExtent2D extent;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<Image_View> image_views;
	};
}