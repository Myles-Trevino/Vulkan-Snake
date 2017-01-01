#pragma once
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include <STB IMAGE/stb_image.h>
#include "Device.hpp"
#include "Command Buffer.hpp"

namespace Oreginum::Vulkan
{
	class Image
	{
	public:
		static constexpr vk::Format SWAPCHAIN_FORMAT{vk::Format::eB8G8R8A8Unorm};
		static constexpr vk::ColorSpaceKHR SWAPCHAIN_COLOR_SPACE{vk::ColorSpaceKHR::eSrgbNonlinear};
		static constexpr vk::Format LINEAR_TEXTURE_FORMAT{vk::Format::eR8G8B8A8Srgb};
		static constexpr vk::Format SRGB_TEXTURE_FORMAT{vk::Format::eR8G8B8A8Unorm};
		static constexpr vk::Format HDR_TEXTURE_FORMAT{vk::Format::eR16G16B16A16Sfloat};
		static constexpr vk::Format DEPTH_FORMAT{vk::Format::eD32Sfloat};
		static const vk::FormatFeatureFlags DEPTH_FEATURES;

		Image(){}
		Image(const Device& device, const vk::Extent2D& extent, vk::ImageUsageFlags usage,
			vk::Format format, vk::ImageAspectFlags aspect);
		Image(const Device& device, const Command_Buffer& temporary_command_buffer,
			const glm::uvec2& resolution, const std::vector<stbi_uc *>& datas,
			vk::Format format, bool cubemap);
		Image(const Device& device, vk::Image image, vk::Format format = SWAPCHAIN_FORMAT,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor)
			: device(&device), image(image), aspect(aspect)
		{ create_image_view(format, aspect); this->image = nullptr; }
		Image *operator=(Image other){ swap(&other); return this; }
		~Image();

		static void transition(const Command_Buffer& temporary_command_buffer,
			const vk::Image& image, vk::ImageAspectFlags aspect, vk::ImageLayout old_layout,
			vk::ImageLayout new_layout, vk::AccessFlags source_access_flags,
			vk::AccessFlags destination_access_flags, uint32_t layers = 1,
			uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
			uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED);
		void transition(const Command_Buffer& temporary_command_buffer,
			vk::ImageLayout old_layout, vk::ImageLayout new_layout,
			vk::AccessFlags source_access_flags, vk::AccessFlags destination_access_flags,
			uint32_t layers = 1, uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
			uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED)
		{
			transition(temporary_command_buffer, image, aspect, old_layout, new_layout,
				source_access_flags, destination_access_flags, source_queue_family_index,
				destination_family_queue_index);
		}

		const vk::Image& get() const { return image; }
		const vk::ImageView& get_view() const { return *image_view; }

	private:
		const Device *device;
		vk::Image image;
		vk::DeviceMemory image_memory;
		std::shared_ptr<vk::ImageView> image_view = std::make_shared<vk::ImageView>();
		vk::ImageAspectFlags aspect;

		void swap(Image *other);
		vk::Image create_image(const Device& device, const vk::Extent2D& extent,
			vk::ImageUsageFlags usage, vk::Format format, vk::ImageAspectFlags aspect =
			vk::ImageAspectFlagBits::eColor, uint32_t layers = 1,
			vk::ImageTiling tiling = vk::ImageTiling::eOptimal, bool cubemap = false);
		vk::DeviceMemory create_and_bind_image_memory(const Device& device, const vk::Image& image,
			vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eDeviceLocal);
		void copy_image(const Command_Buffer& temporary_command_buffer, const vk::Image& source,
			const vk::Image& destination, const glm::uvec2& resolution, uint32_t layer = 1,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor);
		void create_image_view(vk::Format format,
			vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
			uint32_t layers = 1, vk::ImageViewType view_type = vk::ImageViewType::e2D);
	};
}