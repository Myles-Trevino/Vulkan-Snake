#include <map>
#include <set>
#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Device.hpp"

Oreginum::Vulkan::Device::Device(const Instance& instance, const Surface& surface)
	: instance(instance), surface(surface)
{
	select_gpu();
	create_device();
}

void Oreginum::Vulkan::Device::get_gpu_swapchain_information(const vk::PhysicalDevice& gpu)
{
	gpu.getSurfaceCapabilitiesKHR(surface.get(), &surface_capabilities);
	surface_formats = gpu.getSurfaceFormatsKHR(surface.get()).value;
	swapchain_present_modes = gpu.getSurfacePresentModesKHR(surface.get()).value;
}

void Oreginum::Vulkan::Device::get_gpu_information(const vk::PhysicalDevice& gpu)
{
	//General
	gpu.getProperties(&gpu_properties);
	gpu.getFeatures(&gpu_features);

	//Extensions
	supported_gpu_extensions = gpu.enumerateDeviceExtensionProperties().value;

	//Graphics queue
	std::vector<vk::QueueFamilyProperties> queue_family_properties
	{gpu.getQueueFamilyProperties()};

	graphics_queue_family_index = UINT32_MAX;
	for(uint32_t i{}; i < queue_family_properties.size(); ++i)
	{
		if(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			vk::Bool32 surface_supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface.get(), &surface_supported);
			if(surface_supported){ graphics_queue_family_index = i; break; }
		}
	}

	//Swapchain
	get_gpu_swapchain_information(gpu);
}

void Oreginum::Vulkan::Device::select_gpu()
{
	std::vector<vk::PhysicalDevice> gpus{instance.get().enumeratePhysicalDevices().value};

	std::map<int, vk::PhysicalDevice> gpu_ratings;
	for(const auto& g : gpus)
	{
		uint32_t rating{};
		get_gpu_information(g);

		//GPU type
		if(gpu_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) rating += 1;

		//Extensions
		std::set<std::string> required_extensions(gpu_extensions.begin(), gpu_extensions.end());
		for(const auto& e : supported_gpu_extensions)
			required_extensions.erase(e.extensionName);
		if(!required_extensions.empty()) continue;

		//Graphics queue
		if(graphics_queue_family_index == UINT32_MAX) continue;

		//Swapchain minimum image count
		if((surface_capabilities.maxImageCount > 0) && (Core::SWAPCHAIN_MINIMUM_IMAGE_COUNT >
			surface_capabilities.maxImageCount)) continue;

		//Swapchain format
		bool swapchain_format_supported{};
		if(!((surface_formats.size() == 1) &&
			(surface_formats[0].format == vk::Format::eUndefined)))
		{
			for(const auto& f : surface_formats)
				if(f.format == Core::SWAPCHAIN_FORMAT.format && f.colorSpace ==
					Core::SWAPCHAIN_FORMAT.colorSpace) swapchain_format_supported = true;
			if(!swapchain_format_supported) continue;
		}

		//Swapchain present mode
		bool swapchain_present_mode_supported{};
		for(const auto& p : swapchain_present_modes)
			if(p == vk::PresentModeKHR::eMailbox) swapchain_present_mode_supported = true;
		if(!swapchain_present_mode_supported) continue;

		//Depth format
		vk::FormatProperties properties(g.getFormatProperties(Core::DEPTH_FORMAT));
		if((properties.optimalTilingFeatures & Core::DEPTH_FEATURES)
			!= Core::DEPTH_FEATURES) continue;

		gpu_ratings.insert({rating, g});
	}

	if(gpu_ratings.empty())
		Oreginum::Core::error("Could not find a GPU that supports Vulkan sufficiently.");
	gpu = gpu_ratings.begin()->second;
	get_gpu_information(gpu);
}

void Oreginum::Vulkan::Device::create_device()
{
	static constexpr float QUEUE_PRIORITY{1};
	vk::DeviceQueueCreateInfo device_queue_information
	{{}, graphics_queue_family_index, 1, &QUEUE_PRIORITY};

	vk::DeviceCreateInfo device_information{{}, 1, &device_queue_information, 0, nullptr,
		static_cast<uint32_t>(gpu_extensions.size()), gpu_extensions.data(), nullptr};

	if(gpu.createDevice(&device_information, nullptr, &device) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan device.");

	graphics_queue = device.getQueue(graphics_queue_family_index, 0);
}