#pragma once
#include <vector>
#include <array>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Instance.hpp"

namespace Oreginum::Vulkan
{
	class Device
	{
	public:
		Device(){};
		~Device(){ destroy(); };

		void initialize(const Instance *instance);
		void update(){ get_gpu_swapchain_information(gpu); }

		VkDevice get() const { return device; }
		VkPhysicalDevice get_gpu() const { return gpu; }
		const VkSurfaceCapabilitiesKHR& get_surface_capabilities() const
		{ return surface_capabilities; }
		uint32_t get_graphics_queue_family_index() const { return graphics_queue_family_index; }
		VkQueue get_graphics_queue() const { return graphics_queue; }

	private:
		const Instance *instance;
		std::array<const char *, 1> gpu_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		std::vector<VkExtensionProperties> supported_gpu_extensions;
		uint32_t graphics_queue_family_index;
		VkQueue graphics_queue;
		VkPhysicalDeviceProperties gpu_properties;
		VkPhysicalDeviceFeatures gpu_features;
		VkSurfaceCapabilitiesKHR surface_capabilities;
		std::vector<VkSurfaceFormatKHR> surface_formats;
		std::vector<VkPresentModeKHR> swapchain_present_modes;
		VkPhysicalDevice gpu;
		VkDevice device;

		void destroy();

		void get_gpu_swapchain_information(const VkPhysicalDevice& gpu);
		void get_gpu_information(const VkPhysicalDevice& gpu);
		void select_gpu();
		void create_device();
		void get_device_queues();
	};
}