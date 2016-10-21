#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <map>
#include <set>
#include <array>
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include <STB Image/stb_image.h>
#include "Core.hpp"
#include "Window.hpp"
#include "Vulkan.hpp"

namespace
{
	struct Virtual_Frame
	{
		VkCommandBuffer command_buffer;
		VkSemaphore swapchain_image_available_semaphore;
		VkSemaphore rendering_finished_semaphore;
		VkFence fence;
		VkFramebuffer framebuffer;
	};

	static constexpr VkSurfaceFormatKHR SWAPCHAIN_FORMAT
	{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	static constexpr uint32_t SWAPCHAIN_MINIMUM_IMAGE_COUNT{3};
	static constexpr VkFormat IMAGE_FORMAT{VK_FORMAT_B8G8R8A8_UNORM };
	static constexpr VkFormat DEPTH_FORMAT{VK_FORMAT_D32_SFLOAT};

	bool debug;
	const void *vertex_data;
	size_t vertex_data_size;
	const void *uniform_buffer_object;
	size_t uniform_buffer_object_size;

	std::vector<const char *> instance_extensions
	{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
	std::vector<const char *> instance_layers;
	VkInstance instance;
	VkDebugReportCallbackEXT debug_callback;
	VkSurfaceKHR surface;
	std::array<const char *, 1> gpu_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	std::vector<VkExtensionProperties> supported_gpu_extensions;
	uint32_t graphics_queue_family_index;
	VkQueue graphics_queue;
	VkPhysicalDeviceProperties gpu_properties;
	VkPhysicalDeviceFeatures gpu_features;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	std::vector<VkSurfaceFormatKHR> surface_formats;
	std::vector<VkPresentModeKHR> swapchain_present_modes;
	VkPhysicalDevice gpu;
	VkDevice device;
	
	VkExtent2D swapchain_extent;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchain_images;
	VkCommandPool command_pool;

	VkRenderPass render_pass;
	std::vector<VkImageView> swapchain_image_views;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;

	std::array<Virtual_Frame, 3> virtual_frames;

	void create_instance()
	{
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
		instance_information.pApplicationInfo =	&application_information;
		instance_information.enabledLayerCount = static_cast<uint32_t>(instance_layers.size());
		instance_information.ppEnabledLayerNames = instance_layers.data();
		instance_information.enabledExtensionCount =
			static_cast<uint32_t>(instance_extensions.size());
		instance_information.ppEnabledExtensionNames = instance_extensions.data();

		if(vkCreateInstance(&instance_information, nullptr, &instance) != VK_SUCCESS)
			Oreginum::Core::error("Vulkan is not supported sufficiently.");
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback_function(VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT type, uint64_t object, size_t location,
		int32_t code, const char *layer_prefix, const char *message, void *user_data)
	{
		std::cout<<"Vulkan "<<((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ?
			"error" : "warning")<<" from "<<layer_prefix<<":\n\""<<message<<"\"\n";
		return false;
	}

	void create_debug_callback()
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

	void create_surface()
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

	void get_gpu_swapchain_information(const VkPhysicalDevice& gpu = ::gpu)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
		surface_formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface,
			&format_count, surface_formats.data());

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, nullptr);
		swapchain_present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface,
			&present_mode_count, swapchain_present_modes.data());
	}

	void get_gpu_information(const VkPhysicalDevice& gpu = ::gpu)
	{
		//General
		vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
		vkGetPhysicalDeviceFeatures(gpu, &gpu_features);

		//Extensions
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, nullptr);
		supported_gpu_extensions.resize(extension_count);
		vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count,
			supported_gpu_extensions.data());

		//Graphics queue
		uint32_t queue_family_count;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu,
			&queue_family_count, queue_family_properties.data());

		graphics_queue_family_index = UINT32_MAX;
		for(uint32_t i{}; i < queue_family_properties.size(); ++i)
		{
			if(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				VkBool32 surface_supported;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &surface_supported);
				if(surface_supported) { graphics_queue_family_index = i; break; }
			}
		}

		//Swapchain
		get_gpu_swapchain_information(gpu);
	}

	void select_gpu()
	{
		uint32_t gpu_count;
		vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
		std::vector<VkPhysicalDevice> gpus(gpu_count);
		vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

		std::map<int, VkPhysicalDevice> gpu_ratings;
		for(const auto& g : gpus)
		{
			uint32_t rating{};
			get_gpu_information(g);

			//GPU type
			if(gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) rating += 1;

			//Extensions
			std::set<std::string> required_extensions(gpu_extensions.begin(), gpu_extensions.end());
			for(const auto& e : supported_gpu_extensions)
				required_extensions.erase(e.extensionName);
			if(!required_extensions.empty()) continue;

			//Graphics queue
			if(graphics_queue_family_index == UINT32_MAX) continue;

			//Swapchain minimum image count
			if((surface_capabilities.maxImageCount > 0) && (SWAPCHAIN_MINIMUM_IMAGE_COUNT >
				surface_capabilities.maxImageCount)) continue;

			//Swapchain format
			bool swapchain_format_supported{};
			if(!((surface_formats.size() == 1) &&
				(surface_formats[0].format == VK_FORMAT_UNDEFINED)))
			{
				for(const auto& f : surface_formats)
					if(f.format == SWAPCHAIN_FORMAT.format && f.colorSpace ==
						SWAPCHAIN_FORMAT.colorSpace) swapchain_format_supported = true;
				if(!swapchain_format_supported) continue;
			}

			//Swapchain present mode
			bool swapchain_present_mode_supported{};
			for(const auto& p : swapchain_present_modes)
				if(p == VK_PRESENT_MODE_MAILBOX_KHR) swapchain_present_mode_supported = true;
			if(!swapchain_present_mode_supported) continue;

			gpu_ratings.insert({rating, g});
		}

		if(gpu_ratings.empty())
			Oreginum::Core::error("Could not find a GPU that supports Vulkan sufficiently.");
		gpu = gpu_ratings.begin()->second;
		get_gpu_information();
	}

	void create_device()
	{
		static constexpr float QUEUE_PRIORITY{1};
		VkDeviceQueueCreateInfo device_queue_information;
		device_queue_information.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_information.pNext = nullptr;
		device_queue_information.flags = NULL;
		device_queue_information.queueFamilyIndex = graphics_queue_family_index;
		device_queue_information.queueCount = 1;
		device_queue_information.pQueuePriorities = &QUEUE_PRIORITY;

		VkDeviceCreateInfo device_information;
		device_information.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_information.pNext = nullptr;
		device_information.flags = NULL;
		device_information.queueCreateInfoCount = 1;
		device_information.pQueueCreateInfos = &device_queue_information;
		device_information.enabledLayerCount = 0;
		device_information.ppEnabledLayerNames = nullptr;
		device_information.enabledExtensionCount =
			static_cast<uint32_t>(gpu_extensions.size());
		device_information.ppEnabledExtensionNames = gpu_extensions.data();
		device_information.pEnabledFeatures = nullptr;

		if(vkCreateDevice(gpu, &device_information, nullptr, &device) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan device.");
	}

	void get_device_queues()
	{ vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue); }

	void create_semaphore(VkSemaphore *semaphore)
	{
		VkSemaphoreCreateInfo semaphore_information;
		semaphore_information.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_information.pNext = nullptr;
		semaphore_information.flags = NULL;

		if(vkCreateSemaphore(device, &semaphore_information, nullptr, semaphore) != VK_SUCCESS)
			Oreginum::Core::error("Could not create Vulkan semaphores.");
	}

	void create_fence(VkFence *fence, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT)
	{
		VkFenceCreateInfo fence_information;
		fence_information.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_information.pNext = nullptr;
		fence_information.flags = flags;

		if(vkCreateFence(device, &fence_information, nullptr, fence) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan fence.");
	}

	void create_image_view(VkImageView *image_view, VkImage image, VkFormat format,
		VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
		VkImageSubresourceRange subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
		VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY})
	{
		VkImageViewCreateInfo image_view_information;
		image_view_information.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_information.pNext = nullptr;
		image_view_information.flags = NULL;
		image_view_information.image = image;
		image_view_information.viewType = view_type;
		image_view_information.format = format;
		image_view_information.components = components;
		image_view_information.subresourceRange = subresource_range;

		if(vkCreateImageView(device, &image_view_information, nullptr, image_view) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan image view.");
	}

	void create_swapchain()
	{
		get_gpu_swapchain_information();
		swapchain_extent = surface_capabilities.currentExtent;
		VkSwapchainKHR old_swapchain{swapchain};

		VkSwapchainCreateInfoKHR swapchain_information;
		swapchain_information.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_information.pNext = nullptr;
		swapchain_information.flags = NULL;
		swapchain_information.surface = surface;
		swapchain_information.minImageCount =
			std::max(SWAPCHAIN_MINIMUM_IMAGE_COUNT, surface_capabilities.minImageCount);
		swapchain_information.imageFormat = SWAPCHAIN_FORMAT.format;
		swapchain_information.imageColorSpace = SWAPCHAIN_FORMAT.colorSpace;
		swapchain_information.imageExtent = swapchain_extent;
		swapchain_information.imageArrayLayers = 1;
		swapchain_information.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_information.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_information.queueFamilyIndexCount = NULL;
		swapchain_information.pQueueFamilyIndices = nullptr;
		swapchain_information.preTransform = surface_capabilities.currentTransform;
		swapchain_information.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_information.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchain_information.clipped = VK_TRUE;
		swapchain_information.oldSwapchain = old_swapchain;

		if(vkCreateSwapchainKHR(device, &swapchain_information, nullptr, &swapchain) !=
			VK_SUCCESS) Oreginum::Core::error("Could not create Vulkan swapchain.");
		if(old_swapchain) vkDestroySwapchainKHR(device, old_swapchain, nullptr);

		uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
		swapchain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data());

		for(VkImageView& i : swapchain_image_views) vkDestroyImageView(device, i, nullptr);
		swapchain_image_views.resize(swapchain_images.size());
		for(size_t i{}; i < swapchain_image_views.size(); ++i) create_image_view(
			&swapchain_image_views[i], swapchain_images[i], SWAPCHAIN_FORMAT.format);
	}

	void create_render_pass()
	{
		//Attachments
		VkAttachmentDescription color_attachment_description;
		color_attachment_description.flags = NULL;
		color_attachment_description.format = SWAPCHAIN_FORMAT.format;
		color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//Subpasses
		VkAttachmentReference color_attachment_reference;
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description;
		subpass_description.flags = NULL;
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.inputAttachmentCount = 0;
		subpass_description.pInputAttachments = nullptr;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attachment_reference;
		subpass_description.pResolveAttachments = nullptr;
		subpass_description.pDepthStencilAttachment = nullptr;
		subpass_description.preserveAttachmentCount = 0;
		subpass_description.pPreserveAttachments = nullptr;

		//Dependencies
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		//Render pass
		VkRenderPassCreateInfo render_pass_information;
		render_pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_information.pNext = nullptr;
		render_pass_information.flags = NULL;
		render_pass_information.attachmentCount = 1;
		render_pass_information.pAttachments = &color_attachment_description;
		render_pass_information.subpassCount = 1;
		render_pass_information.pSubpasses = &subpass_description;
		render_pass_information.dependencyCount = static_cast<uint32_t>(dependencies.size());
		render_pass_information.pDependencies = dependencies.data();

		vkCreateRenderPass(device, &render_pass_information, nullptr, &render_pass);
	}

	VkShaderModule create_shader_module(const std::string& shader)
	{
		std::ifstream file{"Resources/Shaders/"+shader+".spv", std::ios::ate | std::ios::binary};
		if(!file.is_open()) Oreginum::Core::error("Could not open shader \""+shader+"\".");
		size_t size{static_cast<size_t>(file.tellg())};
		file.seekg(0);
		std::vector<char> data(size);
		file.read(data.data(), size);
		file.close();

		VkShaderModuleCreateInfo shader_module_information;
		shader_module_information.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_information.pNext = nullptr;
		shader_module_information.flags = NULL;
		shader_module_information.codeSize = data.size();
		shader_module_information.pCode = reinterpret_cast<const uint32_t *>(data.data());

		VkShaderModule shader_module;
		if(vkCreateShaderModule(device, &shader_module_information,
			nullptr, &shader_module) != VK_SUCCESS)
			Oreginum::Core::error("Could not create Vulkan shader module \""+shader+"\".");

		return shader_module;
	}

	void create_graphics_pipeline()
	{
		//Shaders
		VkShaderModule vertex_shader_module{create_shader_module("Primitive Vertex")};
		VkShaderModule fragment_shader_module{create_shader_module("Primitive Fragment")};

		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_informations;
		shader_stage_informations[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_informations[0].pNext = nullptr;
		shader_stage_informations[0].flags = NULL;
		shader_stage_informations[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stage_informations[0].module = vertex_shader_module;
		shader_stage_informations[0].pName = "main";
		shader_stage_informations[0].pSpecializationInfo = nullptr;
		shader_stage_informations[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_informations[1].pNext = nullptr;
		shader_stage_informations[1].flags = NULL;
		shader_stage_informations[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stage_informations[1].module = fragment_shader_module;
		shader_stage_informations[1].pName = "main";
		shader_stage_informations[1].pSpecializationInfo = nullptr;

		//Vertex input
		VkVertexInputBindingDescription vertex_input_binding_description;
		vertex_input_binding_description.binding = 0;
		vertex_input_binding_description.stride = sizeof(float)*6;
		vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 2> vertex_attribute_descriptions;
		vertex_attribute_descriptions[0].location = 0;
		vertex_attribute_descriptions[0].binding = vertex_input_binding_description.binding;
		vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_descriptions[0].offset = 0;
		vertex_attribute_descriptions[1].location = 1;
		vertex_attribute_descriptions[1].binding = vertex_input_binding_description.binding;
		vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_descriptions[1].offset = sizeof(float)*3;

		VkPipelineVertexInputStateCreateInfo vertex_input_state_information;
		vertex_input_state_information.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state_information.pNext = nullptr;
		vertex_input_state_information.flags = NULL;
		vertex_input_state_information.vertexBindingDescriptionCount = 1;
		vertex_input_state_information.pVertexBindingDescriptions =
			&vertex_input_binding_description;
		vertex_input_state_information.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(vertex_attribute_descriptions.size());
		vertex_input_state_information.pVertexAttributeDescriptions =
			vertex_attribute_descriptions.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state_information;
		input_assembly_state_information.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state_information.pNext = nullptr;
		input_assembly_state_information.flags = NULL;
		input_assembly_state_information.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		input_assembly_state_information.primitiveRestartEnable = VK_FALSE;

		//Viewport
		VkPipelineViewportStateCreateInfo viewport_state_information;
		viewport_state_information.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_information.pNext = nullptr;
		viewport_state_information.flags = NULL;
		viewport_state_information.viewportCount = 1;
		viewport_state_information.pViewports = nullptr;
		viewport_state_information.scissorCount = 1;
		viewport_state_information.pScissors = nullptr;

		//Rasterization
		VkPipelineRasterizationStateCreateInfo rasterization_state_information;
		rasterization_state_information.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state_information.pNext = nullptr;
		rasterization_state_information.flags = NULL;
		rasterization_state_information.depthClampEnable = VK_FALSE;
		rasterization_state_information.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state_information.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_state_information.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_state_information.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterization_state_information.depthBiasEnable = VK_FALSE;
		rasterization_state_information.depthBiasConstantFactor = 0;
		rasterization_state_information.depthBiasClamp = 0;
		rasterization_state_information.depthBiasSlopeFactor = 0;
		rasterization_state_information.lineWidth = 1;

		//Multisampling
		VkPipelineMultisampleStateCreateInfo multisample_state_information;
		multisample_state_information.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_information.pNext = nullptr;
		multisample_state_information.flags = NULL;
		multisample_state_information.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_information.sampleShadingEnable = VK_FALSE;
		multisample_state_information.minSampleShading = 1;
		multisample_state_information.pSampleMask = nullptr;
		multisample_state_information.alphaToCoverageEnable = VK_FALSE;
		multisample_state_information.alphaToOneEnable = VK_FALSE;

		//Blending
		VkPipelineColorBlendAttachmentState color_blend_attachment_state;
		color_blend_attachment_state.blendEnable = VK_FALSE;
		color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo color_blend_state_information;
		color_blend_state_information.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_information.pNext = nullptr;
		color_blend_state_information.flags = NULL;
		color_blend_state_information.logicOpEnable = VK_FALSE;
		color_blend_state_information.logicOp = VK_LOGIC_OP_COPY;
		color_blend_state_information.attachmentCount = 1;
		color_blend_state_information.pAttachments = &color_blend_attachment_state;
		color_blend_state_information.blendConstants[0] = 0;
		color_blend_state_information.blendConstants[1] = 0;
		color_blend_state_information.blendConstants[2] = 0;
		color_blend_state_information.blendConstants[3] = 0;

		//Dynamic state
		std::array<VkDynamicState, 2> dynamic_states
		{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info;
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.pNext = nullptr;
		dynamic_state_create_info.flags = NULL;
		dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state_create_info.pDynamicStates = dynamic_states.data();

		//Layout
		VkPipelineLayoutCreateInfo layout_information;
		layout_information.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_information.pNext = nullptr;
		layout_information.flags = NULL;
		layout_information.setLayoutCount = 0;
		layout_information.pSetLayouts = nullptr;
		layout_information.pushConstantRangeCount = 0;
		layout_information.pPushConstantRanges = nullptr;

		if(vkCreatePipelineLayout(device, &layout_information, nullptr, &pipeline_layout) !=
			VK_SUCCESS)
		{
			vkDestroyShaderModule(device, fragment_shader_module, nullptr);
			vkDestroyShaderModule(device, vertex_shader_module, nullptr);
			Oreginum::Core::error("Could not create a Vulkan graphics pipeline layout.");
		}

		//Pipeline
		VkGraphicsPipelineCreateInfo pipeline_information;
		pipeline_information.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_information.pNext = nullptr;
		pipeline_information.flags = NULL;
		pipeline_information.stageCount = static_cast<uint32_t>(shader_stage_informations.size());
		pipeline_information.pStages = shader_stage_informations.data();
		pipeline_information.pVertexInputState = &vertex_input_state_information;
		pipeline_information.pInputAssemblyState = &input_assembly_state_information;
		pipeline_information.pTessellationState = nullptr;
		pipeline_information.pViewportState = &viewport_state_information;
		pipeline_information.pRasterizationState = &rasterization_state_information;
		pipeline_information.pMultisampleState = &multisample_state_information;
		pipeline_information.pDepthStencilState = nullptr;
		pipeline_information.pColorBlendState = &color_blend_state_information;
		pipeline_information.pDynamicState = &dynamic_state_create_info;
		pipeline_information.layout = pipeline_layout;
		pipeline_information.renderPass = render_pass;
		pipeline_information.subpass = 0;
		pipeline_information.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_information.basePipelineIndex = -1;

		if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&pipeline_information, nullptr, &pipeline) != VK_SUCCESS)
		{
			vkDestroyShaderModule(device, fragment_shader_module, nullptr);
			vkDestroyShaderModule(device, vertex_shader_module, nullptr);
			Oreginum::Core::error("Could not create a Vulkan graphics pipeline.");
		}

		vkDestroyShaderModule(device, fragment_shader_module, nullptr);
		vkDestroyShaderModule(device, vertex_shader_module, nullptr);
	}

	void create_buffer(VkBuffer *buffer, size_t size, VkBufferUsageFlags usage =
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
		uint32_t queue_family_index_count = 0, const uint32_t *queue_family_indices = nullptr)
	{
		VkBufferCreateInfo buffer_information;
		buffer_information.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_information.pNext = nullptr;
		buffer_information.flags = NULL;
		buffer_information.size = size;
		buffer_information.usage = usage;
		buffer_information.sharingMode = sharing_mode;
		buffer_information.queueFamilyIndexCount = queue_family_index_count;
		buffer_information.pQueueFamilyIndices = queue_family_indices;

		if(vkCreateBuffer(device, &buffer_information, nullptr, buffer) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan buffer.");
	}

	void allocate_buffer_memory(VkDeviceMemory *memory, VkBuffer buffer,
		VkMemoryPropertyFlags memory_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VkDeviceSize offset = 0)
	{
		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

		for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
			if((memory_requirements.memoryTypeBits & (1 << i)) &&
				(memory_properties.memoryTypes[i].propertyFlags & memory_property_flags))
			{
				VkMemoryAllocateInfo memory_information;
				memory_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memory_information.pNext = nullptr;
				memory_information.allocationSize = memory_requirements.size;
				memory_information.memoryTypeIndex = i;

				if(vkAllocateMemory(device, &memory_information, nullptr, memory) != VK_SUCCESS)
					Oreginum::Core::error("Could not allocate memory for a Vulkan buffer.");

				if(vkBindBufferMemory(device, buffer, vertex_buffer_memory, offset) != VK_SUCCESS)
					Oreginum::Core::error("Could not bind memory to a Vulkan vertex buffer.");

				return;
			}
	}

	void upload_data(VkDeviceMemory memory, const void *data, size_t data_size)
	{
		void *vertex_buffer_memory_pointer;
		if(vkMapMemory(device, memory, 0, data_size, NULL, &vertex_buffer_memory_pointer))
			Oreginum::Core::error("Could not map Vulkan vertex buffer memory.");

		memcpy(vertex_buffer_memory_pointer, data, data_size);

		VkMappedMemoryRange mapped_memory_range;
		mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_memory_range.pNext = nullptr;
		mapped_memory_range.memory = memory;
		mapped_memory_range.offset = 0;
		mapped_memory_range.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(device, 1, &mapped_memory_range);
		vkUnmapMemory(device, memory);
	}

	void create_command_pool(VkCommandPool *command_pool, VkCommandPoolCreateFlags flags,
		uint32_t queue_family_index = graphics_queue_family_index)
	{
		VkCommandPoolCreateInfo pool_information;
		pool_information.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_information.pNext = nullptr;
		pool_information.flags = flags;
		pool_information.queueFamilyIndex = queue_family_index;

		if(vkCreateCommandPool(device, &pool_information, nullptr, command_pool) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan command pool.");
	}

	void allocate_command_buffers(VkCommandPool command_pool, VkCommandBuffer *command_buffers,
		uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
	{
		VkCommandBufferAllocateInfo command_buffer_information;
		command_buffer_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_information.pNext = nullptr;
		command_buffer_information.commandPool = command_pool;
		command_buffer_information.level = level;
		command_buffer_information.commandBufferCount = count;

		if(vkAllocateCommandBuffers(device, &command_buffer_information, command_buffers) !=
			VK_SUCCESS) Oreginum::Core::error("Could not allocate a Vulkan command buffer.");
	}

	void create_framebuffer(VkFramebuffer *framebuffer, VkImageView image_view, VkExtent2D extent)
	{
		VkFramebufferCreateInfo framebuffer_information;
		framebuffer_information.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_information.pNext = nullptr;
		framebuffer_information.flags = NULL;
		framebuffer_information.renderPass = render_pass;
		framebuffer_information.attachmentCount = 1;
		framebuffer_information.pAttachments = &image_view;
		framebuffer_information.width = extent.width;
		framebuffer_information.height = extent.height;
		framebuffer_information.layers = 1;

		if(vkCreateFramebuffer(device, &framebuffer_information, nullptr, framebuffer) !=
			VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan framebuffer.");
	}

	void prepare_frame(Virtual_Frame *virtual_frame, VkImageView image_view)
	{
		if(virtual_frame->framebuffer)
			vkDestroyFramebuffer(device, virtual_frame->framebuffer, nullptr);
		create_framebuffer(&virtual_frame->framebuffer, image_view, swapchain_extent);

		VkCommandBufferBeginInfo command_buffer_information;
		command_buffer_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_information.pNext = nullptr;
		command_buffer_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_information.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(virtual_frame->command_buffer, &command_buffer_information);

		VkClearValue clear_value{1, .8f, .4f, 0};
		VkRenderPassBeginInfo render_pass_begin_information;
		render_pass_begin_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_information.pNext = nullptr;
		render_pass_begin_information.renderPass = render_pass;
		render_pass_begin_information.framebuffer = virtual_frame->framebuffer;
		render_pass_begin_information.renderArea = {{0, 0}, swapchain_extent};
		render_pass_begin_information.clearValueCount = 1;
		render_pass_begin_information.pClearValues = &clear_value;

		vkCmdBeginRenderPass(virtual_frame->command_buffer, &render_pass_begin_information,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(virtual_frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport{0, 0, static_cast<float>(swapchain_extent.width),
			static_cast<float>(swapchain_extent.height), 0, 1};
		VkRect2D scissor{{0, 0}, swapchain_extent};

		vkCmdSetViewport(virtual_frame->command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(virtual_frame->command_buffer, 0, 1, &scissor);

		VkDeviceSize offset{};
		vkCmdBindVertexBuffers(virtual_frame->command_buffer, 0, 1, &vertex_buffer, &offset);

		vkCmdDraw(virtual_frame->command_buffer, 4, 1, 0, 0);

		vkCmdEndRenderPass(virtual_frame->command_buffer);

		if(vkEndCommandBuffer(virtual_frame->command_buffer) != VK_SUCCESS)
			Oreginum::Core::error("Could not record a Vulkan command buffer.");
	}

	void recreate_swapchain()
	{
		if(device) vkDeviceWaitIdle(device);
		create_swapchain();
	}
}

void Oreginum::Vulkan::initialize(const void *vertex_data, size_t vertex_data_size,
	const void *uniform_buffer_object, size_t uniform_buffer_object_size, bool debug)
{
	::debug = debug;
	::vertex_data = vertex_data;
	::vertex_data_size = vertex_data_size;
	::uniform_buffer_object = uniform_buffer_object;
	::uniform_buffer_object_size = uniform_buffer_object_size;

	create_instance();
	if(debug) create_debug_callback();
	create_surface();
	select_gpu();
	create_device();
	get_device_queues();
	create_swapchain();

	create_render_pass();
	create_graphics_pipeline();
	create_buffer(&vertex_buffer, vertex_data_size);
	allocate_buffer_memory(&vertex_buffer_memory, vertex_buffer);
	upload_data(vertex_buffer_memory, vertex_data, vertex_data_size);
	create_command_pool(&command_pool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	for(auto& v : virtual_frames)
	{
		allocate_command_buffers(command_pool, &v.command_buffer);
		create_semaphore(&v.swapchain_image_available_semaphore);
		create_semaphore(&v.rendering_finished_semaphore);
		create_fence(&v.fence);
	}
}

void Oreginum::Vulkan::destroy()
{
	if(device) vkDeviceWaitIdle(device);
	for(Virtual_Frame& v : virtual_frames)
	{
		if(v.framebuffer) vkDestroyFramebuffer(device, v.framebuffer, nullptr);
		if(v.fence) vkDestroyFence(device, v.fence, nullptr);
		if(v.rendering_finished_semaphore)
			vkDestroySemaphore(device, v.rendering_finished_semaphore, nullptr);
		if(v.swapchain_image_available_semaphore)
			vkDestroySemaphore(device, v.swapchain_image_available_semaphore, nullptr);
		if(v.command_buffer) vkFreeCommandBuffers(device, command_pool, 1, &v.command_buffer);
	}

	if(command_pool) vkDestroyCommandPool(device, command_pool, nullptr);
	if(vertex_buffer_memory) vkFreeMemory(device, vertex_buffer_memory, nullptr);
	if(vertex_buffer) vkDestroyBuffer(device, vertex_buffer, nullptr);
	if(pipeline) vkDestroyPipeline(device, pipeline, nullptr);
	if(pipeline_layout) vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
	if(render_pass) vkDestroyRenderPass(device, render_pass, nullptr);

	for(VkImageView& i : swapchain_image_views) vkDestroyImageView(device, i, nullptr);
	if(swapchain) vkDestroySwapchainKHR(device, swapchain, nullptr);
	if(device) vkDestroyDevice(device, nullptr);
	if(surface) vkDestroySurfaceKHR(instance, surface, nullptr);
	if(debug_callback)
	{
		auto fvkDestroyDebugReportCallbackEXT{(PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT")};
		fvkDestroyDebugReportCallbackEXT(instance, debug_callback, nullptr);
	}
	if(instance) vkDestroyInstance(instance, nullptr);
}

void Oreginum::Vulkan::render()
{
	//Recreate swapchain on resize
	if(Window::was_resized()) recreate_swapchain();

	//Virtual frame
	static size_t virtual_frame_index{};
	virtual_frame_index = (virtual_frame_index+1)%virtual_frames.size();
	Virtual_Frame *virtual_frame = &virtual_frames[virtual_frame_index];
	vkWaitForFences(device, 1, &virtual_frame->fence, VK_FALSE, UINT64_MAX);
	vkResetFences(device, 1, &virtual_frame->fence);

	//Swapchain image
	uint32_t image_index;
	VkResult result{vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, 
		virtual_frame->swapchain_image_available_semaphore, VK_NULL_HANDLE, &image_index)};
	if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) recreate_swapchain();
	else if(result != VK_SUCCESS)
		Oreginum::Core::error("Could not aquire a Vulkan swapchain image.");

	//Prepare frame
	prepare_frame(virtual_frame, swapchain_image_views[image_index]);

	//Submit render buffer
	VkPipelineStageFlags wait_destination_stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_information;
	submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_information.pNext = nullptr;
	submit_information.waitSemaphoreCount = 1;
	submit_information.pWaitSemaphores = &virtual_frame->swapchain_image_available_semaphore;
	submit_information.pWaitDstStageMask = &wait_destination_stage_mask;
	submit_information.commandBufferCount = 1;
	submit_information.pCommandBuffers = &virtual_frame->command_buffer;
	submit_information.signalSemaphoreCount = 1;
	submit_information.pSignalSemaphores = &virtual_frame->rendering_finished_semaphore;

	if(vkQueueSubmit(graphics_queue, 1, &submit_information, virtual_frame->fence) != VK_SUCCESS)
		Oreginum::Core::error("Could not submit Vulkan render queue.");

	//Submit present queue
	VkPresentInfoKHR present_information;
	present_information.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_information.pNext = nullptr;
	present_information.waitSemaphoreCount = 1;
	present_information.pWaitSemaphores = &virtual_frame->rendering_finished_semaphore;
	present_information.swapchainCount = 1;
	present_information.pSwapchains = &swapchain;
	present_information.pImageIndices = &image_index;
	present_information.pResults = nullptr;

	result = vkQueuePresentKHR(graphics_queue, &present_information);
	if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) recreate_swapchain();
	else if(result != VK_SUCCESS)
		Oreginum::Core::error("Could not submit Vulkan presentation queue.");
}