#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include "../Oreginum/Model.hpp"
#include "Command Pool.hpp"

namespace Oreginum::Vulkan::Core
{
	static constexpr vk::SurfaceFormatKHR SWAPCHAIN_FORMAT
	{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
	static constexpr uint32_t SWAPCHAIN_MINIMUM_IMAGE_COUNT{3};
	static constexpr vk::Format IMAGE_FORMAT{vk::Format::eB8G8R8A8Unorm};
	static constexpr vk::Format DEPTH_FORMAT{vk::Format::eD32Sfloat};
	static const vk::FormatFeatureFlags DEPTH_FEATURES
	{vk::FormatFeatureFlagBits::eDepthStencilAttachment};

	void initialize(const Oreginum::Model& model, const void *uniform_buffer_object,
		size_t uniform_buffer_object_size, bool debug = false);
	void destroy();

	uint32_t find_memory(const Device& device, uint32_t type,
		vk::MemoryPropertyFlags properties);
	const vk::CommandBuffer& begin_single_time_commands(
		const Device& device, const Command_Pool& command_pool);
	void end_single_time_commands(const Device& device,
		const Command_Pool& command_pool, const vk::CommandBuffer& command_buffer);
	void render();
}