#pragma once
#include "Device.hpp"
#include "Command Buffer.hpp"

namespace Oreginum::Vulkan
{
	class Image
	{
	public:
		static constexpr vk::Format FORMAT{vk::Format::eB8G8R8A8Unorm};
		static constexpr vk::Format DEPTH_FORMAT{vk::Format::eD32Sfloat};
		static const vk::FormatFeatureFlags DEPTH_FEATURES;

		Image(const Device& device, const vk::Extent2D& extent, vk::Format format = FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment);
		Image(const Device& device, vk::Image image, vk::Format format = FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor)
			: device(&device), image(image), aspect(aspect)
		{ create_image_view(format, aspect); this->image = nullptr; }
		Image *Image::operator=(Image other){ swap(&other); return this; }
		~Image();

		void transition(const Command_Pool& temporary_command_pool, vk::ImageLayout old_layout,
			vk::ImageLayout new_layout, vk::AccessFlags source_access_flags,
			vk::AccessFlags destination_access_flags,
			uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
			uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED);

		const vk::Image& get() const { return image; }
		const vk::ImageView& get_view() const { return *image_view; }

	private:
		const Device *device;
		vk::Image image;
		vk::DeviceMemory image_memory;
		std::shared_ptr<vk::ImageView> image_view = std::make_shared<vk::ImageView>();
		vk::ImageAspectFlags aspect;

		void swap(Image *other);
		void create_image_view(vk::Format format = FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
	};
}