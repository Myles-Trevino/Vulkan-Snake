#include <map>
#include <set>
#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Device.hpp"

Oreginum::Vulkan::Device::~Device(){ if(device) vkDestroyDevice(device, nullptr); }

void Oreginum::Vulkan::Device::initialize(const Instance *instance)
{
	this->instance = instance;

	select_gpu();
	create_device();
	get_device_queues();
}

void Oreginum::Vulkan::Device::get_gpu_swapchain_information(const VkPhysicalDevice& gpu)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu,
		instance->get_surface(), &surface_capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, instance->get_surface(), &format_count, nullptr);
	surface_formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, instance->get_surface(),
		&format_count, surface_formats.data());

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu,
		instance->get_surface(), &present_mode_count, nullptr);
	swapchain_present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, instance->get_surface(),
		&present_mode_count, swapchain_present_modes.data());
}

void Oreginum::Vulkan::Device::get_gpu_information(const VkPhysicalDevice& gpu)
{
	//General
	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
	vkGetPhysicalDeviceFeatures(gpu, &gpu_features);

	//Extensions
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, nullptr);
	supported_gpu_extensions.resize(extension_count);
	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count,
		supported_gpu_extensions.data());

	//Graphics queue
	uint32_t queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu,
		&queue_family_count, queue_family_properties.data());

	graphics_queue_family_index = UINT32_MAX;
	for(uint32_t i{}; i < queue_family_properties.size(); ++i)
	{
		if(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			VkBool32 surface_supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i,
				instance->get_surface(), &surface_supported);
			if(surface_supported) { graphics_queue_family_index = i; break; }
		}
	}

	//Swapchain
	get_gpu_swapchain_information(gpu);
}

void Oreginum::Vulkan::Device::select_gpu()
{
	uint32_t gpu_count;
	vkEnumeratePhysicalDevices(instance->get(), &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> gpus(gpu_count);
	vkEnumeratePhysicalDevices(instance->get(), &gpu_count, gpus.data());

	std::map<int, VkPhysicalDevice> gpu_ratings;
	for(const auto& g : gpus)
	{
		uint32_t rating{};
		get_gpu_information(g);

		//GPU type
		if(gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) rating += 1;

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
			(surface_formats[0].format == VK_FORMAT_UNDEFINED)))
		{
			for(const auto& f : surface_formats)
				if(f.format == Core::SWAPCHAIN_FORMAT.format && f.colorSpace ==
					Core::SWAPCHAIN_FORMAT.colorSpace) swapchain_format_supported = true;
			if(!swapchain_format_supported) continue;
		}

		//Swapchain present mode
		bool swapchain_present_mode_supported{};
		for(const auto& p : swapchain_present_modes)
			if(p == VK_PRESENT_MODE_MAILBOX_KHR) swapchain_present_mode_supported = true;
		if(!swapchain_present_mode_supported) continue;

		//Depth format
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(g, Core::DEPTH_FORMAT, &properties);
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
	VkDeviceQueueCreateInfo device_queue_information;
	device_queue_information.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_information.pNext = nullptr;
	device_queue_information.flags = NULL;
	device_queue_information.queueFamilyIndex = graphics_queue_family_index;
	device_queue_information.queueCount = 1;
	device_queue_information.pQueuePriorities = &QUEUE_PRIORITY;

	VkDeviceCreateInfo device_information;
	device_information.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_information.pNext = nullptr;
	device_information.flags = NULL;
	device_information.queueCreateInfoCount = 1;
	device_information.pQueueCreateInfos = &device_queue_information;
	device_information.enabledLayerCount = 0;
	device_information.ppEnabledLayerNames = nullptr;
	device_information.enabledExtensionCount =
		static_cast<uint32_t>(gpu_extensions.size());
	device_information.ppEnabledExtensionNames = gpu_extensions.data();
	device_information.pEnabledFeatures = nullptr;

	if(vkCreateDevice(gpu, &device_information, nullptr, &device) != VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan device.");
}

void Oreginum::Vulkan::Device::get_device_queues()
{ vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue); }