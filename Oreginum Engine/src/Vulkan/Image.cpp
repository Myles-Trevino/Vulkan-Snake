#include "../Oreginum/Core.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

const vk::FormatFeatureFlags Oreginum::Vulkan::Image::DEPTH_FEATURES
{vk::FormatFeatureFlagBits::eDepthStencilAttachment};

Oreginum::Vulkan::Image::Image(const Device& device, const vk::Extent2D& extent,
	vk::Format format, vk::ImageAspectFlags aspect, vk::ImageUsageFlags usage)
	: device(&device), aspect(aspect)
{
	//Create image
	vk::ImageCreateInfo image_information{{}, vk::ImageType::e2D, format, {extent.width,
		extent.height, 1}, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
		usage, vk::SharingMode::eExclusive, 0, nullptr, vk::ImageLayout::ePreinitialized};
	if(device.get().createImage(&image_information, nullptr, &image) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan image.");

	//Allocate image memory
	vk::MemoryRequirements memory_requirements(device.get().getImageMemoryRequirements(image));
	vk::MemoryAllocateInfo memory_information{memory_requirements.size, 
		Buffer::find_memory(device, memory_requirements.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eDeviceLocal)};
	if(device.get().allocateMemory(&memory_information,
		nullptr, &image_memory) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate memory for a Vulkan image.");

	//Bind image memory
	device.get().bindImageMemory(image, image_memory, 0);

	//Create image view
	create_image_view(format, aspect);
}

Oreginum::Vulkan::Image::~Image()
{
	if(!image_view.unique()) return;
	if(*image_view) device->get().destroyImageView(*image_view);
	if(image_memory) device->get().freeMemory(image_memory);
	if(image) device->get().destroyImage(image);
}

void Oreginum::Vulkan::Image::swap(Image *other)
{
	std::swap(this->device, other->device);
	std::swap(this->image, other->image);
	std::swap(this->image_memory, other->image_memory);
	std::swap(this->image_view, other->image_view);
	std::swap(this->aspect, other->aspect);
}

void Oreginum::Vulkan::Image::create_image_view(vk::Format format, vk::ImageAspectFlags aspect)
{
	vk::ImageViewCreateInfo image_view_information
	{{}, image, vk::ImageViewType::e2D, format, {}, {aspect, 0, 1, 0, 1}};
	if(device->get().createImageView(&image_view_information, nullptr, image_view.get()) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan image view.");
}

void Oreginum::Vulkan::Image::transition(const Command_Pool& temporary_command_pool,
	vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::AccessFlags source_access_flags,
	vk::AccessFlags destination_access_flags, uint32_t source_queue_family_index,
	uint32_t destination_family_queue_index)
{
	vk::ImageMemoryBarrier image_memory_barrier{source_access_flags, destination_access_flags,
		old_layout, new_layout, source_queue_family_index, destination_family_queue_index,
		image, {aspect, 0, 1, 0, 1}};

	vk::CommandBuffer temporary_command_buffer
	{Command_Buffer::begin_single_time_commands(*device, temporary_command_pool)};
	temporary_command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlagBits{}, 0,
		nullptr, 0, nullptr, 1, &image_memory_barrier);

	Command_Buffer::end_single_time_commands(*device,
		temporary_command_pool, temporary_command_buffer);
}