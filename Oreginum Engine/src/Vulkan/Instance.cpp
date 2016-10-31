#include <iostream>
#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Instance.hpp"

void Oreginum::Vulkan::Instance::destroy()
{
	if(surface) vkDestroySurfaceKHR(instance, surface, nullptr);
	if(debug_callback)
	{
		auto fvkDestroyDebugReportCallbackEXT{(PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT")};
		fvkDestroyDebugReportCallbackEXT(instance, debug_callback, nullptr);
	}
	if(instance) vkDestroyInstance(instance, nullptr);
}

void Oreginum::Vulkan::Instance::initialize(bool debug)
{
	destroy();

	VkApplicationInfo application_information;
	application_information.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_information.pNext = nullptr;
	application_information.pApplicationName = nullptr;
	application_information.applicationVersion = NULL;
	application_information.pEngineName = "Oreginum Engine";
	application_information.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	application_information.apiVersion = VK_MAKE_VERSION(1, 0, 26);

	if(debug) instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME),
		instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");

	VkInstanceCreateInfo instance_information;
	instance_information.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_information.pNext = nullptr;
	instance_information.flags = NULL;
	instance_information.pApplicationInfo = &application_information;
	instance_information.enabledLayerCount = static_cast<uint32_t>(instance_layers.size());
	instance_information.ppEnabledLayerNames = instance_layers.data();
	instance_information.enabledExtensionCount =
		static_cast<uint32_t>(instance_extensions.size());
	instance_information.ppEnabledExtensionNames = instance_extensions.data();

	if(vkCreateInstance(&instance_information, nullptr, &instance) != VK_SUCCESS)
		Oreginum::Core::error("Vulkan is not supported sufficiently.");

	if(debug) create_debug_callback();
	create_surface();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback_function(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT type, uint64_t object, size_t location,
	int32_t code, const char *layer_prefix, const char *message, void *user_data)
{
	std::cout<<"Vulkan "<<((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ?
		"error" : "warning")<<" from "<<layer_prefix<<":\n\""<<message<<"\"\n";
	return false;
}

void Oreginum::Vulkan::Instance::create_debug_callback()
{
	VkDebugReportCallbackCreateInfoEXT debug_callback_information;
	debug_callback_information.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_callback_information.pNext = nullptr;
	debug_callback_information.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debug_callback_information.pfnCallback = debug_callback_function;
	debug_callback_information.pUserData = nullptr;

	auto fvkCreateDebugReportCallbackEXT{(PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT")};

	if(!fvkCreateDebugReportCallbackEXT || fvkCreateDebugReportCallbackEXT(instance,
		&debug_callback_information, nullptr, &debug_callback) != VK_SUCCESS)
		Oreginum::Core::error("Could not initialize Vulkan debugging.");
}

void Oreginum::Vulkan::Instance::create_surface()
{
	VkWin32SurfaceCreateInfoKHR surface_information;
	surface_information.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_information.pNext = nullptr;
	surface_information.flags = NULL;
	surface_information.hinstance = Oreginum::Window::get_instance();
	surface_information.hwnd = Oreginum::Window::get();

	if(vkCreateWin32SurfaceKHR(instance, &surface_information, nullptr, &surface) !=
		VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan surface.");
}