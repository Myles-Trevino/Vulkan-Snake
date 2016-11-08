#pragma once
#include "Core.hpp"
#include "Device.hpp"
#include "Command Buffer.hpp"

namespace Oreginum::Vulkan
{
	class Image
	{
	public:
		Image(const Device& device, const Command_Buffer& command_buffer,
			const vk::Extent2D& extent,
			vk::Format format = Oreginum::Vulkan::Core::IMAGE_FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment);
		Image(const Device& device, const Command_Buffer& command_buffer, vk::Image image,
			vk::Format format = Oreginum::Vulkan::Core::IMAGE_FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor)
			: device(device), command_buffer(command_buffer)
		{ create_image_view(format, aspect); }
		~Image();

		void transition(vk::Format format, vk::ImageLayout old_layout,
			vk::ImageLayout new_layout, vk::AccessFlags source_access_flags,
			vk::AccessFlags destination_access_flags,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
			uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
			uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED);

		const vk::Image& get() const { return image; }
		const vk::ImageView& get_view() const { return *image_view; }

	private:
		const Device& device;
		const Command_Buffer& command_buffer;

		vk::Image image;
		vk::DeviceMemory image_memory;
		std::shared_ptr<vk::ImageView> image_view = std::make_shared<vk::ImageView>();

		void create_image_view(vk::Format format = Core::IMAGE_FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
	};
}