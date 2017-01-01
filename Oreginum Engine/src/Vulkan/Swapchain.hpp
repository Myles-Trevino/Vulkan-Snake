#pragma once
#include "Instance.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "Image.hpp"

namespace Oreginum::Vulkan
{
	class Swapchain
	{
	public:
		static constexpr uint32_t MINIMUM_IMAGE_COUNT{3};

		Swapchain(){};
		Swapchain(const Instance& instance, const Surface& surface, Device *device)
			: instance(&instance), surface(&surface), device(device)
		{ initialize(instance, surface, device); }
		Swapchain *operator=(Swapchain other){ swap(&other); return this; }
		~Swapchain();

		void reinitialize(Device *device)
		{ initialize(*instance, *surface, device); }

		const vk::SwapchainKHR& get() const { return *swapchain; }
		const std::vector<Image>& get_images() const { return images; }
		const vk::Extent2D& get_extent() const { return extent; }

	private:
		const Device *device;
		const Surface *surface;
		const Instance *instance;
		vk::Extent2D extent;
		std::shared_ptr<vk::SwapchainKHR> swapchain = std::make_shared<vk::SwapchainKHR>();
		std::vector<Image> images;

		void swap(Swapchain *other);
		void initialize(const Instance& instance, const Surface& surface, Device *device);
	};
}