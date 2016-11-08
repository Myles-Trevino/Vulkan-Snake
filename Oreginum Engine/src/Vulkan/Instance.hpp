#pragma once
#include <vector>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.hpp>

namespace Oreginum::Vulkan
{
	class Instance
	{
	public:
		Instance(bool debug = false);
		~Instance();

		const vk::Instance& get() const { return instance; }

	private:
		std::vector<const char *> instance_extensions
		{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
		std::vector<const char *> instance_layers{};
		vk::Instance instance;
		vk::DebugReportCallbackEXT debug_callback;

		void create_debug_callback();
	};
}