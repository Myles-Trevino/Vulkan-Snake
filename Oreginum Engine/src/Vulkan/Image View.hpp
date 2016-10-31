#pragma once
#include <memory>
#include "Device.hpp"
#include <iostream>
#include <string>

namespace Oreginum::Vulkan
{
	class Image_View
	{
	public:
		Image_View(){}
		~Image_View(){ destroy(); }

		void initialize(const Device *device, VkImage image, VkFormat format,
			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
			VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
			VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, 
			VK_COMPONENT_SWIZZLE_IDENTITY});

		VkImageView get() const { return *image_view; }

	private:
		const Device *device;
		std::shared_ptr<VkImageView> image_view = std::make_shared<VkImageView>();

		void destroy();
	};
}