#pragma once
#include <vector>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>

namespace Oreginum::Vulkan
{
	class Instance
	{
	public:
		Instance(){};
		~Instance(){ destroy(); }

		void initialize(bool debug = false);

		VkInstance get() const { return instance; }
		VkSurfaceKHR get_surface() const { return surface; }

	private:
		std::vector<const char *> instance_extensions
		{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
		std::vector<const char *> instance_layers;
		VkInstance instance;
		VkDebugReportCallbackEXT debug_callback;
		VkSurfaceKHR surface;

		void destroy();

		void create_debug_callback();
		void create_surface();
	};
}