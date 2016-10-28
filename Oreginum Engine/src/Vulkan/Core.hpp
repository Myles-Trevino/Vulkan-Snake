#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include "../Oreginum/Model.hpp"

namespace Oreginum::Vulkan::Core
{
	static constexpr VkSurfaceFormatKHR SWAPCHAIN_FORMAT
	{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	static constexpr uint32_t SWAPCHAIN_MINIMUM_IMAGE_COUNT{3};
	static constexpr VkFormat IMAGE_FORMAT{VK_FORMAT_B8G8R8A8_UNORM};
	static constexpr VkFormat DEPTH_FORMAT{VK_FORMAT_D32_SFLOAT};
	static constexpr VkFormatFeatureFlags DEPTH_FEATURES
	{VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT};

	void initialize(const Oreginum::Model& model, const void *uniform_buffer_object,
		size_t uniform_buffer_object_size, bool debug = false);
	void destroy();

	uint32_t find_memory(uint32_t type, VkMemoryPropertyFlags properties);
	VkCommandBuffer begin_single_time_commands();
	void end_single_time_commands(VkCommandBuffer command_buffer);

	void render();
}