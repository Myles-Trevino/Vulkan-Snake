#include "../Oreginum/Core.hpp"
#include "Image View.hpp"

void Oreginum::Vulkan::Image_View::destroy()
{
	if(image_view.use_count() == 1 && *image_view)
		vkDestroyImageView(device->get(), *image_view, nullptr);
}

void Oreginum::Vulkan::Image_View::initialize(const Device *device, VkImage image,
	VkFormat format, VkImageAspectFlags aspect, VkImageViewType view_type,
	VkComponentMapping components)
{
	this->device = device;
	if(*image_view) vkDestroyImageView(device->get(), *image_view, nullptr);

	VkImageViewCreateInfo image_view_information;
	image_view_information.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_information.pNext = nullptr;
	image_view_information.flags = NULL;
	image_view_information.image = image;
	image_view_information.viewType = view_type;
	image_view_information.format = format;
	image_view_information.components = components;
	image_view_information.subresourceRange = {aspect, 0, 1, 0, 1};

	if(vkCreateImageView(device->get(), &image_view_information, nullptr, &*image_view) !=
		VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan image view.");
}