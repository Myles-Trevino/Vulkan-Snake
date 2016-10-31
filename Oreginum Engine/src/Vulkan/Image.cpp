#include "../Oreginum/Core.hpp"
#include "Image.hpp"

void Oreginum::Vulkan::Image::destroy()
{
	if(image_memory) vkFreeMemory(device->get(), image_memory, nullptr);
	if(image) vkDestroyImage(device->get(), image, nullptr);
}

void Oreginum::Vulkan::Image::initialize(const Device *device, const glm::uvec2& resolution,
	VkFormat format, VkImageAspectFlags aspect, VkImageUsageFlags usage, VkImageTiling tiling,
	VkMemoryPropertyFlags properties, uint32_t mip_levels, VkSampleCountFlagBits samples,
	VkImageType type, VkImageViewType view_type, uint32_t array_layers,
	VkSharingMode sharing_mode, const std::vector<uint32_t>& queue_family_indices,
	VkComponentMapping components)
{
	this->device = device;
	destroy();

	VkImageCreateInfo image_information;
	image_information.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_information.pNext = nullptr;
	image_information.flags = NULL;
	image_information.imageType = type;
	image_information.format = format;
	image_information.extent = {resolution.x, resolution.y, 1};
	image_information.mipLevels = mip_levels;
	image_information.arrayLayers = array_layers;
	image_information.samples = samples;
	image_information.tiling = tiling;
	image_information.usage = usage;
	image_information.sharingMode = sharing_mode;
	image_information.queueFamilyIndexCount =
		static_cast<uint32_t>(queue_family_indices.size());
	image_information.pQueueFamilyIndices = queue_family_indices.data();
	image_information.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	if(vkCreateImage(device->get(), &image_information, nullptr, &image) != VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan image.");

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device->get(), image, &memory_requirements);

	VkMemoryAllocateInfo memory_information;
	memory_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_information.pNext = nullptr;
	memory_information.allocationSize = memory_requirements.size;
	memory_information.memoryTypeIndex =
		Oreginum::Vulkan::Core::find_memory(memory_requirements.memoryTypeBits, properties);

	if(vkAllocateMemory(device->get(), &memory_information, nullptr, &image_memory) !=
		VK_SUCCESS) Oreginum::Core::error("Could not allocate memory for a Vulkan image.");

	vkBindImageMemory(device->get(), image, image_memory, 0);

	image_view.initialize(device, image, format, aspect, view_type, components);
}

void Oreginum::Vulkan::Image::transition(VkFormat format, VkImageLayout old_layout,
	VkImageLayout new_layout, VkAccessFlags source_access_flags,
	VkAccessFlags destination_access_flags, VkImageAspectFlags aspect,
	uint32_t source_queue_family_index, uint32_t destination_family_queue_index)
{
	VkCommandBuffer command_buffer{Oreginum::Vulkan::Core::begin_single_time_commands()};

	VkImageMemoryBarrier image_memory_barrier;
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.pNext = nullptr;
	image_memory_barrier.srcAccessMask = source_access_flags;
	image_memory_barrier.dstAccessMask = destination_access_flags;
	image_memory_barrier.oldLayout = old_layout;
	image_memory_barrier.newLayout = new_layout;
	image_memory_barrier.srcQueueFamilyIndex = source_queue_family_index;
	image_memory_barrier.dstQueueFamilyIndex = destination_family_queue_index;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange = {aspect, 0, 1, 0, 1};

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
		&image_memory_barrier);

	Oreginum::Vulkan::Core::end_single_time_commands(command_buffer);
}