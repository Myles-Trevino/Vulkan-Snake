#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <array>
#include "Error.hpp"
#include "Vulkan.hpp"

Vulkan::Vulkan(const Window& window, const std::string& program_title,
	const glm::ivec3& program_version, const std::string& engine_title,
	const glm::ivec3& engine_version, const glm::ivec3& vulkan_version,
	const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions,
	const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions,
	const std::vector<float>& vertices, bool debug)
	: WINDOW(window), DEBUG(debug), vertex_binding_descriptions(vertex_binding_descriptions),
	vertex_attribute_descriptions(vertex_attribute_descriptions), vertices(vertices)
{
	create_instance(window, program_title, program_version,
		engine_title, engine_version, vulkan_version);
	if(DEBUG) create_debug_callback();
	create_surface();
	select_gpu();
	create_device();
	create_swapchain();
	create_image_views();
	create_render_pass();
	create_graphics_pipeline();
	create_framebuffers();
	create_command_pool();
	create_vertex_buffer();
	create_command_buffers();
	create_semaphores();
}

void Vulkan::render()
{
	if(WINDOW.was_resized() && WINDOW.is_visible()) recreate_swapchain();

	uint32_t image_index;
	VkResult result{vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(),
		image_available_semaphore, VK_NULL_HANDLE, &image_index)};

	if(result == VK_ERROR_OUT_OF_DATE_KHR){ recreate_swapchain(); return; }
	else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		error("Could not acquire Vulkan swap chain image.");

	VkSemaphore wait_semaphores[]{image_available_semaphore};
	VkPipelineStageFlags wait_stages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submit_information{};
	submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_information.waitSemaphoreCount = 1;
	submit_information.pWaitSemaphores = wait_semaphores;
	submit_information.pWaitDstStageMask = wait_stages;
	submit_information.commandBufferCount = 1;
	submit_information.pCommandBuffers = &command_buffers[image_index];
	VkSemaphore signal_semaphores[]{render_finished_semaphore};
	submit_information.signalSemaphoreCount = 1;
	submit_information.pSignalSemaphores = signal_semaphores;
	if(vkQueueSubmit(graphics_queue, 1,
		&submit_information, VK_NULL_HANDLE) != VK_SUCCESS)
		error("Could not submit Vulkan command buffer.");
	VkSwapchainKHR swapchains[]{swapchain};
	VkPresentInfoKHR present_information{};
	present_information.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_information.waitSemaphoreCount = 1;
	present_information.pWaitSemaphores = signal_semaphores;
	present_information.swapchainCount = 1;
	present_information.pSwapchains = swapchains;
	present_information.pImageIndices = &image_index;
	present_information.pResults = nullptr;

	result = vkQueuePresentKHR(present_queue, &present_information);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) recreate_swapchain();
	else if(result != VK_SUCCESS) error("Could not present Vulkan swap chain image.");
}

void Vulkan::create_instance(const Window& window, const std::string& program_title,
	const glm::ivec3& program_version, const std::string& engine_title,
	const glm::ivec3& engine_version, const glm::ivec3& vulkan_version)
{
	VkApplicationInfo application_information{};
	application_information.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_information.pApplicationName = program_title.c_str();
	application_information.applicationVersion =
		VK_MAKE_VERSION(program_version.x, program_version.y, program_version.z);
	application_information.pEngineName = engine_title.c_str();
	application_information.engineVersion =
		VK_MAKE_VERSION(engine_version.x, engine_version.y, engine_version.z);
	application_information.apiVersion =
		VK_MAKE_VERSION(vulkan_version.x, vulkan_version.y, vulkan_version.z);

	std::vector<const char *> extensions{VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
	if(DEBUG) extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	std::vector<const char *> layers;
	if(DEBUG) layers.push_back("VK_LAYER_LUNARG_standard_validation");

	VkInstanceCreateInfo instance_information{};
	instance_information.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_information.pApplicationInfo = &application_information;
	instance_information.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_information.ppEnabledExtensionNames = extensions.data();
	instance_information.enabledLayerCount = static_cast<uint32_t>(layers.size());
	instance_information.ppEnabledLayerNames = layers.data();

	if(vkCreateInstance(&instance_information, nullptr, &instance) != VK_SUCCESS)
		error("Could not create a Vulkan instance.");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debug_callback_function(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT type, uint64_t object, size_t location,
	int32_t code, const char *layer_prefix, const char *message, void *user_data)
{
	std::cout<<"Vulkan "<<((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ?
		"error" : "warning")<<" from "<<layer_prefix<<":\n\""<<message<<'\"';
	return false;
}

void Vulkan::create_debug_callback()
{
	VkDebugReportCallbackCreateInfoEXT debug_information{};
	debug_information.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_information.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	debug_information.pfnCallback = debug_callback_function;

	auto fvkCreateDebugReportCallbackEXT{(PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT")};

	if(!fvkCreateDebugReportCallbackEXT || fvkCreateDebugReportCallbackEXT(
		instance, &debug_information, nullptr, &debug_callback) != VK_SUCCESS)
		error("Could not initialize Vulkan debugging.");
}

void Vulkan::destroy_debug_callback(VkInstance instance, VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks *allocator)
{
	auto fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT(instance, callback, allocator);
}

void Vulkan::create_surface()
{
	VkWin32SurfaceCreateInfoKHR surface_information{};
	surface_information.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_information.pNext = NULL;
	surface_information.flags = 0;
	surface_information.hinstance = WINDOW.get_instance();
	surface_information.hwnd = WINDOW.get();

	if(vkCreateWin32SurfaceKHR(instance, &surface_information, nullptr, &surface) != VK_SUCCESS)
		error("Could not create Vulkan surface.");
}

void Vulkan::get_gpu_information(const VkPhysicalDevice& gpu)
{
	uint32_t queue_family_count{};
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families{queue_family_count};
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_families.data());

	graphics_queue_index = -1, present_queue_index = -1;
	for(int i{}; i < queue_families.size(); ++i)
	{
		if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphics_queue_index = i;
		VkBool32 surface_supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, graphics_queue_index,
			surface, &surface_supported);
		if(surface_supported) present_queue_index = i;
		if(graphics_queue_index == present_queue_index) break;
	}
}

void Vulkan::get_swapchain_information(const VkPhysicalDevice& gpu)
{
	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
	vkGetPhysicalDeviceFeatures(gpu, &gpu_features);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &swapchain_capabilities);

	uint32_t format_count{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
	swapchain_formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface,
		&format_count, swapchain_formats.data());

	uint32_t present_mode_count{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, nullptr);
	swapchain_present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface,
		&present_mode_count, swapchain_present_modes.data());
}

void Vulkan::select_gpu()
{
	uint32_t gpu_count{};
	vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> gpus{gpu_count};
	vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());

	std::map<int, VkPhysicalDevice> rated_gpus;
	for(const auto& g : gpus)
	{
		int rating{};

		uint32_t extension_count{};
		vkEnumerateDeviceExtensionProperties(g, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> supported_extensions{extension_count};
		vkEnumerateDeviceExtensionProperties(g, nullptr,
			&extension_count, supported_extensions.data());
		std::set<std::string> required_extensions(gpu_extensions.begin(), gpu_extensions.end());
		for(const auto& e : supported_extensions)
			required_extensions.erase(e.extensionName);
		if(!required_extensions.empty()) continue;

		get_gpu_information(g);
		get_swapchain_information(g);

		if(!(swapchain_formats.size() == 1 && swapchain_formats[0].format == VK_FORMAT_UNDEFINED))
		{
			bool swapchain_format_supported{};
			for(const auto& f : swapchain_formats)
				if(f.format == VK_FORMAT_B8G8R8A8_UNORM &&
					f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					swapchain_format_supported = true;
			if(!swapchain_format_supported) continue;
		}

		bool swapchain_present_mode_supported{};
		for(const auto& p : swapchain_present_modes)
			if(p == VK_PRESENT_MODE_MAILBOX_KHR) swapchain_present_mode_supported = true;
		if(!swapchain_present_mode_supported) continue;

		if(graphics_queue_index == -1 || present_queue_index == -1) continue;
		else if(graphics_queue_index == present_queue_index) rating += 1;
		if(gpu_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) rating += 2;

		rated_gpus.insert({rating, g});
	}

	if(rated_gpus.empty()) error("Could not find a GPU that supports Vulkan sufficiently.");
	gpu = rated_gpus.begin()->second;
	get_gpu_information(gpu);
}

void Vulkan::create_device()
{
	std::vector<VkDeviceQueueCreateInfo> queue_informations;
	std::set<uint32_t> unique_queues{graphics_queue_index, present_queue_index};
	for(auto q : unique_queues)
	{
		static constexpr float QUEUE_PRIORITY{1};
		VkDeviceQueueCreateInfo queue_information{};
		queue_information.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_information.queueFamilyIndex = q;
		queue_information.queueCount = 1;
		queue_information.pQueuePriorities = &QUEUE_PRIORITY;
		queue_informations.push_back(queue_information);
	}

	VkDeviceCreateInfo device_information{};
	device_information.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_information.pQueueCreateInfos = queue_informations.data();
	device_information.queueCreateInfoCount = static_cast<uint32_t>(queue_informations.size());
	device_information.pEnabledFeatures = &gpu_features;
	device_information.enabledExtensionCount = static_cast<uint32_t>(gpu_extensions.size());
	device_information.ppEnabledExtensionNames = gpu_extensions.data();

	if(vkCreateDevice(gpu, &device_information, nullptr, &device) != VK_SUCCESS)
		error("Could not create a Vulkan device.");

	vkGetDeviceQueue(device, graphics_queue_index, NULL, &graphics_queue);
	vkGetDeviceQueue(device, present_queue_index, NULL, &present_queue);
}

void Vulkan::create_swapchain()
{
	VkSwapchainKHR new_swapchain;
	get_swapchain_information(gpu);
	swapchain_extent = swapchain_capabilities.currentExtent;
	uint32_t image_count{swapchain_capabilities.minImageCount+1};
	uint32_t queue_indices[]{graphics_queue_index, present_queue_index};
	VkSwapchainCreateInfoKHR swapchain_information{};
	swapchain_information.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_information.surface = surface;
	swapchain_information.minImageCount = image_count;
	swapchain_information.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	swapchain_information.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchain_information.imageExtent = swapchain_extent;
	swapchain_information.imageArrayLayers = 1;
	swapchain_information.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if(graphics_queue_index == present_queue_index)
		swapchain_information.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		swapchain_information.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_information.queueFamilyIndexCount = 2;
		swapchain_information.pQueueFamilyIndices = queue_indices;
	}
	swapchain_information.preTransform = swapchain_capabilities.currentTransform;
	swapchain_information.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_information.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapchain_information.clipped = VK_TRUE;
	swapchain_information.oldSwapchain = swapchain;
	if(vkCreateSwapchainKHR(device, &swapchain_information, nullptr, &new_swapchain) != VK_SUCCESS)
		error("Could not create Vulkan swapchain.");
	*swapchain.replace() = new_swapchain;

	vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
	swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());
}

void Vulkan::create_image_views()
{
	swapchain_image_views.resize(swapchain_images.size(),
		Vulkan_Deleter<VkImageView>{device, vkDestroyImageView});
	for(uint32_t i{}; i < swapchain_images.size(); ++i)
	{
		VkImageViewCreateInfo image_view_information{};
		image_view_information.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_information.image = swapchain_images[i];
		image_view_information.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_information.format = VK_FORMAT_B8G8R8A8_UNORM;
		image_view_information.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_information.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_information.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_information.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_information.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_information.subresourceRange.baseMipLevel = 0;
		image_view_information.subresourceRange.levelCount = 1;
		image_view_information.subresourceRange.baseArrayLayer = 0;
		image_view_information.subresourceRange.layerCount = 1;
		if(vkCreateImageView(device, &image_view_information,
			nullptr, &swapchain_image_views[i]) != VK_SUCCESS)
			error("Could not create a Vulkan image view.");
	}
}

void Vulkan::create_render_pass()
{
	VkAttachmentDescription color_attachment{};
	color_attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo pass_information{};
	pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	pass_information.attachmentCount = 1;
	pass_information.pAttachments = &color_attachment;
	pass_information.subpassCount = 1;
	pass_information.pSubpasses = &subpass;
	pass_information.dependencyCount = 1;
	pass_information.pDependencies = &dependency;

	if(vkCreateRenderPass(device, &pass_information, nullptr, &render_pass) != VK_SUCCESS)
		error("Could not create Vulkan render pass.");
}

Vulkan_Deleter<VkShaderModule> Vulkan::create_shader(const std::string& shader)
{
	std::ifstream file{"Resources/Shaders/"+shader+".spv", std::ios::ate | std::ios::binary};
	if(!file.is_open()) error("Could not open shader \""+shader+"\".");
	size_t size{static_cast<size_t>(file.tellg())};
	file.seekg(0);
	std::vector<char> data(size);
	file.read(data.data(), size);
	file.close();

	VkShaderModuleCreateInfo shader_information{};
	shader_information.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_information.codeSize = data.size();
	shader_information.pCode = (uint32_t*)data.data();

	Vulkan_Deleter<VkShaderModule> module{device, vkDestroyShaderModule};
	if(vkCreateShaderModule(device, &shader_information, nullptr,
		&module) != VK_SUCCESS) error("Could not create shader \""+shader+"\".");

	return module;
}

void Vulkan::create_graphics_pipeline()
{
	Vulkan_Deleter<VkShaderModule> primitive_vertex{create_shader("Primitive Vertex")};
	Vulkan_Deleter<VkShaderModule> primitive_fragment{create_shader("Primitive Fragment")};

	VkPipelineShaderStageCreateInfo vertex_shader_information{};
	vertex_shader_information.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_information.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_information.module = primitive_vertex;
	vertex_shader_information.pName = "main";

	VkPipelineShaderStageCreateInfo fragment_shader_information{};
	fragment_shader_information.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_information.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_information.module = primitive_fragment;
	fragment_shader_information.pName = "main";

	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages
	{vertex_shader_information, fragment_shader_information};

	VkPipelineVertexInputStateCreateInfo vertex_input_information{};
	vertex_input_information.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_information.vertexBindingDescriptionCount =
		static_cast<uint32_t>(vertex_binding_descriptions.size());
	vertex_input_information.pVertexBindingDescriptions = vertex_binding_descriptions.data();
	vertex_input_information.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(vertex_attribute_descriptions.size());
	vertex_input_information.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_information{};
	input_assembly_information.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_information.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_information.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain_extent.width);
	viewport.height = static_cast<float>(swapchain_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapchain_extent;

	VkPipelineViewportStateCreateInfo viewport_state_information{};
	viewport_state_information.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_information.viewportCount = 1;
	viewport_state_information.pViewports = &viewport;
	viewport_state_information.scissorCount = 1;
	viewport_state_information.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer_information{};
	rasterizer_information.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_information.depthClampEnable = VK_FALSE;
	rasterizer_information.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_information.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_information.lineWidth = 1.0f;
	rasterizer_information.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_information.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer_information.depthBiasEnable = VK_FALSE;
	rasterizer_information.depthBiasConstantFactor = 0.0f;
	rasterizer_information.depthBiasClamp = 0.0f;
	rasterizer_information.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling_information{};
	multisampling_information.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_information.sampleShadingEnable = VK_FALSE;
	multisampling_information.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling_information.minSampleShading = 1.0f;
	multisampling_information.pSampleMask = nullptr;
	multisampling_information.alphaToCoverageEnable = VK_FALSE;
	multisampling_information.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blending_attachment_information{};
	color_blending_attachment_information.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blending_attachment_information.blendEnable = VK_FALSE;
	color_blending_attachment_information.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blending_attachment_information.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blending_attachment_information.colorBlendOp = VK_BLEND_OP_ADD;
	color_blending_attachment_information.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blending_attachment_information.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blending_attachment_information.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending_information{};
	color_blending_information.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending_information.logicOpEnable = VK_FALSE;
	color_blending_information.logicOp = VK_LOGIC_OP_COPY;
	color_blending_information.attachmentCount = 1;
	color_blending_information.pAttachments = &color_blending_attachment_information;
	color_blending_information.blendConstants[0] = 0.0f;
	color_blending_information.blendConstants[1] = 0.0f;
	color_blending_information.blendConstants[2] = 0.0f;
	color_blending_information.blendConstants[3] = 0.0f;

	static constexpr std::array<VkDynamicState, 2> DYNAMIC_STATES
	{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
	VkPipelineDynamicStateCreateInfo dynamic_state_information{};
	dynamic_state_information.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_information.dynamicStateCount = static_cast<uint32_t>(DYNAMIC_STATES.size());
	dynamic_state_information.pDynamicStates = DYNAMIC_STATES.data();

	VkPipelineLayoutCreateInfo pipeline_layout_information{};
	pipeline_layout_information.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_information.setLayoutCount = 0;
	pipeline_layout_information.pSetLayouts = nullptr;
	pipeline_layout_information.pushConstantRangeCount = 0;
	pipeline_layout_information.pPushConstantRanges = 0;

	if(vkCreatePipelineLayout(device, &pipeline_layout_information, nullptr, &pipeline_layout)
		!= VK_SUCCESS) error("Could not create a Vulkan graphics pipeline layout.");

	VkGraphicsPipelineCreateInfo pipeline_information{};
	pipeline_information.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_information.stageCount = static_cast<uint32_t>(shader_stages.size());
	pipeline_information.pStages = shader_stages.data();
	pipeline_information.pVertexInputState = &vertex_input_information;
	pipeline_information.pInputAssemblyState = &input_assembly_information;
	pipeline_information.pViewportState = &viewport_state_information;
	pipeline_information.pRasterizationState = &rasterizer_information;
	pipeline_information.pMultisampleState = &multisampling_information;
	pipeline_information.pDepthStencilState = nullptr;
	pipeline_information.pColorBlendState = &color_blending_information;
	pipeline_information.pDynamicState = nullptr;
	pipeline_information.layout = pipeline_layout;
	pipeline_information.renderPass = render_pass;
	pipeline_information.subpass = 0;
	pipeline_information.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_information.basePipelineIndex = -1;

	if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
		&pipeline_information, nullptr, &pipeline) != VK_SUCCESS)
		error("Could not create a Vulkan graphics pipeline.");
}

void Vulkan::create_framebuffers()
{
	framebuffers.resize(swapchain_image_views.size(),
		Vulkan_Deleter<VkFramebuffer>{device, vkDestroyFramebuffer});
	for(size_t i{}; i < swapchain_image_views.size(); ++i)
	{
		VkImageView attachments[]{swapchain_image_views[i]};

		VkFramebufferCreateInfo framebuffer_information{};
		framebuffer_information.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_information.renderPass = render_pass;
		framebuffer_information.attachmentCount = 1;
		framebuffer_information.pAttachments = attachments;
		framebuffer_information.width = swapchain_extent.width;
		framebuffer_information.height = swapchain_extent.height;
		framebuffer_information.layers = 1;

		if(vkCreateFramebuffer(device, &framebuffer_information, nullptr,
			&framebuffers[i]) != VK_SUCCESS) error("Could not create a Vulkan framebuffer.");
	}
}

void Vulkan::create_command_pool()
{
	VkCommandPoolCreateInfo pool_information{};
	pool_information.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_information.queueFamilyIndex = graphics_queue_index;
	pool_information.flags = 0;
	if(vkCreateCommandPool(device, &pool_information, nullptr, &command_pool)
		!= VK_SUCCESS) error("Could not create Vulkan command pool.");
}

uint32_t Vulkan::find_memory(uint32_t type, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
	for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
		if((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags
			& properties) == properties) return i;
	error("Your GPU ran out of memory.");
}

void Vulkan::create_vertex_buffer()
{
	VkBufferCreateInfo vertex_buffer_information{};
	vertex_buffer_information.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertex_buffer_information.size = sizeof(vertices[0])*vertices.size();
	vertex_buffer_information.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertex_buffer_information.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if(vkCreateBuffer(device, &vertex_buffer_information, nullptr, vertex_buffer.replace())
		!= VK_SUCCESS) error("Could not create a Vulkan vertex buffer");

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device, vertex_buffer, &memory_requirements);
	VkMemoryAllocateInfo memory_allocation_information{};
	memory_allocation_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocation_information.allocationSize = memory_requirements.size;
	memory_allocation_information.memoryTypeIndex = find_memory(memory_requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if(vkAllocateMemory(device, &memory_allocation_information,
		nullptr, vertex_buffer_memory.replace()) != VK_SUCCESS)
		error("Could not allocate memory for the Vulkan vertex buffer.");
	vkBindBufferMemory(device, vertex_buffer, vertex_buffer_memory, 0);
	void *data;
	vkMapMemory(device, vertex_buffer_memory, 0, vertex_buffer_information.size, 0, &data);
	memcpy(data, vertices.data(), (size_t)vertex_buffer_information.size);
	vkUnmapMemory(device, vertex_buffer_memory);
}

void Vulkan::create_command_buffers()
{
	command_buffers.resize(framebuffers.size());
	VkCommandBufferAllocateInfo buffer_allocation_information{};
	buffer_allocation_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	buffer_allocation_information.commandPool = command_pool;
	buffer_allocation_information.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	buffer_allocation_information.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
	if(vkAllocateCommandBuffers(device, &buffer_allocation_information, command_buffers.data())
		!= VK_SUCCESS) error("Could not create Vulkan command buffers.");

	for(size_t i{}; i < command_buffers.size(); i++)
	{
		VkCommandBufferBeginInfo buffer_begin_information{};
		buffer_begin_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		buffer_begin_information.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		buffer_begin_information.pInheritanceInfo = nullptr;
		vkBeginCommandBuffer(command_buffers[i], &buffer_begin_information);

		VkClearValue clearColor{0.0f, 0.0f, 0.0f, 1.0f};
		VkRenderPassBeginInfo render_pass_information{};
		render_pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_information.renderPass = render_pass;
		render_pass_information.framebuffer = framebuffers[i];
		render_pass_information.renderArea.offset = {0, 0};
		render_pass_information.renderArea.extent = swapchain_extent;
		render_pass_information.clearValueCount = 1;
		render_pass_information.pClearValues = &clearColor;
		vkCmdBeginRenderPass(command_buffers[i], &render_pass_information,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkBuffer vertex_buffers[]{vertex_buffer};
		VkDeviceSize offsets[]{0};
		vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
		vkCmdDraw(command_buffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

		vkCmdEndRenderPass(command_buffers[i]);
		if(vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS)
			error("Could not record commands to a Vulkan command buffer.");
	}
}

void Vulkan::create_semaphores()
{
	VkSemaphoreCreateInfo semaphore_information{};
	semaphore_information.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if(vkCreateSemaphore(device, &semaphore_information, nullptr, &image_available_semaphore)
		!= VK_SUCCESS || vkCreateSemaphore(device, &semaphore_information, nullptr,
			&render_finished_semaphore) != VK_SUCCESS) error("Could not create Vulkan semaphores.");
}

void Vulkan::recreate_swapchain()
{
	vkDeviceWaitIdle(device);
	create_swapchain();
	create_image_views();
	create_render_pass();
	create_graphics_pipeline();
	create_framebuffers();
	create_command_buffers();
}