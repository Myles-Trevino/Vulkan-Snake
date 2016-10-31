#include <algorithm>
#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Swapchain.hpp"

Oreginum::Vulkan::Swapchain::~Swapchain()
{ if(swapchain) vkDestroySwapchainKHR(device->get(), swapchain, nullptr); }

void Oreginum::Vulkan::Swapchain::initialize(const Instance *instance, Device *device)
{
	this->device = device;

	device->update();
	extent = device->get_surface_capabilities().currentExtent;
	VkSwapchainKHR old_swapchain{swapchain};

	VkSwapchainCreateInfoKHR swapchain_information;
	swapchain_information.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_information.pNext = nullptr;
	swapchain_information.flags = NULL;
	swapchain_information.surface = instance->get_surface();
	swapchain_information.minImageCount =
		std::max(Oreginum::Vulkan::Core::SWAPCHAIN_MINIMUM_IMAGE_COUNT,
			device->get_surface_capabilities().minImageCount);
	swapchain_information.imageFormat = Core::SWAPCHAIN_FORMAT.format;
	swapchain_information.imageColorSpace = Core::SWAPCHAIN_FORMAT.colorSpace;
	swapchain_information.imageExtent = extent;
	swapchain_information.imageArrayLayers = 1;
	swapchain_information.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_information.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_information.queueFamilyIndexCount = NULL;
	swapchain_information.pQueueFamilyIndices = nullptr;
	swapchain_information.preTransform = device->get_surface_capabilities().currentTransform;
	swapchain_information.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_information.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapchain_information.clipped = VK_TRUE;
	swapchain_information.oldSwapchain = old_swapchain;

	if(vkCreateSwapchainKHR(device->get(), &swapchain_information, nullptr, &swapchain) !=
		VK_SUCCESS) Oreginum::Core::error("Could not create Vulkan swapchain.");
	if(old_swapchain) vkDestroySwapchainKHR(device->get(), old_swapchain, nullptr);

	uint32_t swapchain_image_count;
	vkGetSwapchainImagesKHR(device->get(), swapchain, &swapchain_image_count, nullptr);
	images.resize(swapchain_image_count);
	vkGetSwapchainImagesKHR(device->get(), swapchain, &swapchain_image_count, images.data());

	image_views.resize(images.size());
	for(size_t i{}; i < image_views.size(); ++i)
		image_views[i].initialize(device, images[i], 
			Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.format);
}