#pragma once
#include <vector>
#include <array>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#include "Instance.hpp"
#include "Surface.hpp"

namespace Oreginum::Vulkan
{
	class Device
	{
	public:
		Device(const Instance& instance, const Surface& surface);
		~Device(){ device.destroy(); }

		void update(){ get_gpu_swapchain_information(gpu); }

		const vk::Device& get() const { return device; }
		const vk::PhysicalDevice& get_gpu() const { return gpu; }
		const vk::SurfaceCapabilitiesKHR& get_surface_capabilities() const
		{ return surface_capabilities; }
		uint32_t get_graphics_queue_family_index() const { return graphics_queue_family_index; }
		const vk::Queue& get_graphics_queue() const { return graphics_queue; }

	private:
		const Instance& instance;
		const Surface& surface;

		std::array<const char *, 1> gpu_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		std::vector<vk::ExtensionProperties> supported_gpu_extensions;
		uint32_t graphics_queue_family_index;
		vk::Queue graphics_queue;
		vk::PhysicalDeviceProperties gpu_properties;
		vk::PhysicalDeviceFeatures gpu_features;
		vk::SurfaceCapabilitiesKHR surface_capabilities;
		std::vector<vk::SurfaceFormatKHR> surface_formats;
		std::vector<vk::PresentModeKHR> swapchain_present_modes;
		vk::PhysicalDevice gpu;
		vk::Device device;

		void get_gpu_swapchain_information(const vk::PhysicalDevice& gpu);
		void get_gpu_information(const vk::PhysicalDevice& gpu);
		void select_gpu();
		void create_device();
	};
}