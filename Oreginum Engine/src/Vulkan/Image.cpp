#include "../Oreginum/Core.hpp"
#include "../Oreginum/Renderer.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

const vk::FormatFeatureFlags Oreginum::Vulkan::Image::DEPTH_FEATURES
{vk::FormatFeatureFlagBits::eDepthStencilAttachment};

Oreginum::Vulkan::Image::Image(const Device& device, const Command_Buffer& temporary_command_buffer,
	const glm::uvec2& resolution, const std::vector<stbi_uc *>& datas, vk::Format format,
	bool cubemap) : device(&device), aspect(vk::ImageAspectFlagBits::eColor)
{
	uint32_t layers = static_cast<uint32_t>(datas.size());

	std::vector<std::pair<vk::Image, vk::DeviceMemory>> stages;
	for(const stbi_uc *d : datas)
	{
		//Create stage image
		vk::Image stage_image{create_image(device, {resolution.x, resolution.y},
			vk::ImageUsageFlagBits::eTransferSrc, format, aspect, 1, vk::ImageTiling::eLinear)};
		vk::DeviceMemory stage_image_memory{create_and_bind_image_memory(
			device, stage_image, vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent)};

		//Copy datas to stage image
		uint32_t size{resolution.x*resolution.y*4};
		auto result{device.get().mapMemory(stage_image_memory, 0, size)};
		if(result.result!=vk::Result::eSuccess)
			Core::error("Could not map Vulkan image stage memory.");

		vk::ImageSubresource image_subresource{aspect, 0, 0};
		vk::SubresourceLayout stage_image_layout{
			device.get().getImageSubresourceLayout(stage_image, image_subresource)};

		if(stage_image_layout.rowPitch == resolution.x*4) std::memcpy(result.value, d, size);
		else
		{
			uint8_t *data_bytes{reinterpret_cast<uint8_t *>(result.value)};
			for(uint32_t i{}; i < resolution.y; ++i)
				memcpy(&data_bytes[i*stage_image_layout.rowPitch],
					&d[i*resolution.y*4], resolution.x*4);
		}

		device.get().unmapMemory(stage_image_memory);
		stages.push_back({stage_image, stage_image_memory});
	}

	//Create image
	image = create_image(device, {resolution.x, resolution.y},
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		format, vk::ImageAspectFlagBits::eColor, layers, vk::ImageTiling::eOptimal, cubemap);
	image_memory = create_and_bind_image_memory(device, image);

	transition(temporary_command_buffer, image, aspect,
		vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal,
		vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferWrite, layers);

	//Copy stage images to image
	for(int i{}; i < stages.size(); ++i)
	{
		transition(temporary_command_buffer, stages[i].first, aspect,
			vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferSrcOptimal,
			vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead);
		
		copy_image(temporary_command_buffer, stages[i].first, image, resolution, i);
	}

	//Transition and create image view
	transition(temporary_command_buffer, image, aspect,
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, layers);

	create_image_view(format, aspect, layers, cubemap ?
		vk::ImageViewType::eCube : vk::ImageViewType::e2D);
}

Oreginum::Vulkan::Image::Image(const Device& device, const vk::Extent2D& extent,
	vk::ImageUsageFlags usage, vk::Format format, vk::ImageAspectFlags aspect)
	: device(&device), aspect(aspect)
{
	image = create_image(device, extent, usage, format, aspect);
	image_memory = create_and_bind_image_memory(device, image);
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
	std::swap(device, other->device);
	std::swap(image, other->image);
	std::swap(image_memory, other->image_memory);
	std::swap(image_view, other->image_view);
	std::swap(aspect, other->aspect);
}

void Oreginum::Vulkan::Image::transition(const Command_Buffer& temporary_command_buffer,
	const vk::Image& image, vk::ImageAspectFlags aspect, vk::ImageLayout old_layout,
	vk::ImageLayout new_layout, vk::AccessFlags source_access_flags,
	vk::AccessFlags destination_access_flags, uint32_t layers, uint32_t source_queue_family_index,
	uint32_t destination_family_queue_index)
{
	vk::ImageMemoryBarrier image_memory_barrier{source_access_flags, destination_access_flags,
		old_layout, new_layout, source_queue_family_index, destination_family_queue_index,
		image, {aspect, 0, 1, 0, layers}};
	temporary_command_buffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	temporary_command_buffer.get().pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	temporary_command_buffer.end_and_submit();
}

vk::Image Oreginum::Vulkan::Image::create_image(const Device& device, const vk::Extent2D& extent,
	vk::ImageUsageFlags usage, vk::Format format, vk::ImageAspectFlags aspect, uint32_t layers,
	vk::ImageTiling tiling, bool cubemap)
{
	vk::Image image;
	vk::ImageCreateInfo image_information{cubemap ? vk::ImageCreateFlagBits::eCubeCompatible :
		vk::ImageCreateFlags{}, vk::ImageType::e2D, format, {extent.width, extent.height, 1},
		1, layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive,
		0, nullptr, vk::ImageLayout::ePreinitialized};

	if(device.get().createImage(&image_information, nullptr, &image) != vk::Result::eSuccess)
		Core::error("Could not create a Vulkan image.");
	return image;
}

vk::DeviceMemory Oreginum::Vulkan::Image::create_and_bind_image_memory(const Device& device,
	const vk::Image& image, vk::MemoryPropertyFlags flags)
{
	vk::DeviceMemory image_memory;
	vk::MemoryRequirements memory_requirements(device.get().getImageMemoryRequirements(image));
	vk::MemoryAllocateInfo memory_information{memory_requirements.size,
		Buffer::find_memory(device, memory_requirements.memoryTypeBits, flags)};
	if(device.get().allocateMemory(&memory_information,
		nullptr, &image_memory) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate memory for a Vulkan image.");
	device.get().bindImageMemory(image, image_memory, 0);
	return image_memory;
}

void Oreginum::Vulkan::Image::copy_image(const Command_Buffer& temporary_command_buffer,
	const vk::Image& source, const vk::Image& destination, const glm::uvec2& resolution,
	uint32_t layer, vk::ImageAspectFlags aspect)
{
	vk::ImageSubresourceLayers source_subresource{aspect, 0, 0, 1};
	vk::ImageSubresourceLayers destination_subresource{aspect, 0, layer, 1};
	vk::ImageCopy region{source_subresource, {}, destination_subresource,
		{}, {resolution.x, resolution.y, 1}};
	temporary_command_buffer.begin();
	temporary_command_buffer.get().copyImage(
		source, vk::ImageLayout::eTransferSrcOptimal,
		destination, vk::ImageLayout::eTransferDstOptimal, region);
	temporary_command_buffer.end_and_submit();
}

void Oreginum::Vulkan::Image::create_image_view(vk::Format format, 
	vk::ImageAspectFlags aspect, uint32_t layers, vk::ImageViewType view_type)
{
	vk::ImageViewCreateInfo image_view_information
	{{}, image, view_type, format, {}, {aspect, 0, 1, 0, layers}};
	if(device->get().createImageView(&image_view_information, nullptr, image_view.get()) !=
		vk::Result::eSuccess) Oreginum::Core::error("Could not create a Vulkan image view.");
}