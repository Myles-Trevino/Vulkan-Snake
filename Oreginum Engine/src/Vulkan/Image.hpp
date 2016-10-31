#pragma once
#include "Core.hpp"
#include "Device.hpp"
#include "Image View.hpp"

namespace Oreginum::Vulkan
{
	class Image
	{
	public:
		Image(){};
		~Image(){ destroy(); };

		void initialize(const Device *device, const glm::uvec2& resolution,
			VkFormat format = Oreginum::Vulkan::Core::IMAGE_FORMAT,
			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
			VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			uint32_t mip_levels = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
			VkImageType type = VK_IMAGE_TYPE_2D,
			VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, uint32_t array_layers = 1,
			VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
			const std::vector<uint32_t>& queue_family_indices = {},
			VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, 
			VK_COMPONENT_SWIZZLE_IDENTITY});

		void transition(VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout,
			VkAccessFlags source_access_flags, VkAccessFlags destination_access_flags,
			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
			uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
			uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED);

		VkImage get() const { return image; }
		VkDeviceMemory get_memory() const { return image_memory; }
		const Image_View& get_view() const { return image_view; }

	private:
		const Device *device;

		VkImage image;
		VkDeviceMemory image_memory;
		Image_View image_view;

		void destroy();
	};
}