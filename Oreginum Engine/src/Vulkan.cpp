#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <array>
#define STB_IMAGE_IMPLEMENTATION
#include <STB Image/stb_image.h>
#include "Error.hpp"
#include "Vulkan.hpp"

Vulkan::Vulkan(const Window& window, const std::string& program_title,
	const glm::ivec3& program_version, const std::string& engine_title,
	const glm::ivec3& engine_version, const glm::ivec3& vulkan_version,
	const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions,
	const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions,
	const std::vector<float>& vertices, const std::vector<uint16_t>& indices,
	const void *const uniform_buffer_object, const size_t uniform_buffer_object_size,
	const std::string& texture, bool debug)
	: WINDOW(window), DEBUG(debug), vertex_binding_descriptions(vertex_binding_descriptions),
	vertex_attribute_descriptions(vertex_attribute_descriptions), vertices(vertices),
	indices(indices), uniform_buffer_object(uniform_buffer_object),
	uniform_buffer_object_size(uniform_buffer_object_size)
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
	create_descriptor_set_layout();
	create_graphics_pipeline();
	create_command_pool();
	create_depth_resources();
	create_framebuffers();
	create_texture(texture);
	create_texture_view();
	create_texture_sampler();
	create_vertex_buffer();
	create_index_buffer();
	create_uniform_buffer();
	create_descriptor_pool();
	create_descriptor_set();
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
		error("Could not acquire a Vulkan swapchain image.");

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
		error("Could not submit a Vulkan command buffer.");
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
	else if(result != VK_SUCCESS) error("Could not present a Vulkan swapchain image.");
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
		error("Vulkan is not supported sufficiently.");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debug_callback_function(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT type, uint64_t object, size_t location,
	int32_t code, const char *layer_prefix, const char *message, void *user_data)
{
	std::cout<<"Vulkan "<<((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ?
		"error" : "warning")<<" from "<<layer_prefix<<":\n\""<<message<<"\"\n";
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
		create_image_view(swapchain_images[i], VK_FORMAT_B8G8R8A8_UNORM, 
			VK_IMAGE_ASPECT_COLOR_BIT, swapchain_image_views[i]);
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

	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = DEPTH_FORMAT;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_reference{};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments{color_attachment, depth_attachment};
	VkRenderPassCreateInfo pass_information{};
	pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	pass_information.attachmentCount = static_cast<uint32_t>(attachments.size());
	pass_information.pAttachments = attachments.data();
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

void Vulkan::create_descriptor_set_layout()
{
	VkDescriptorSetLayoutBinding uniform_buffer_object_layout_binding{};
	uniform_buffer_object_layout_binding.binding = 0;
	uniform_buffer_object_layout_binding.descriptorCount = 1;
	uniform_buffer_object_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_object_layout_binding.pImmutableSamplers = nullptr;
	uniform_buffer_object_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding sampler_layout_binding{};
	sampler_layout_binding.binding = 1;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_binding.pImmutableSamplers = nullptr;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings
	{uniform_buffer_object_layout_binding, sampler_layout_binding};
	VkDescriptorSetLayoutCreateInfo layout_information{};
	layout_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_information.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_information.pBindings = bindings.data();

	if(vkCreateDescriptorSetLayout(device, &layout_information, nullptr,
		descriptor_set_layout.replace()) != VK_SUCCESS)
		error("Could not create Vulkan descriptor set layout.");
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
	rasterizer_information.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	VkPipelineDepthStencilStateCreateInfo depth_stencil_information{};
	depth_stencil_information.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_information.depthTestEnable = VK_TRUE;
	depth_stencil_information.depthWriteEnable = VK_TRUE;
	depth_stencil_information.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_information.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_information.minDepthBounds = 0.0f;
	depth_stencil_information.maxDepthBounds = 1.0f;
	depth_stencil_information.stencilTestEnable = VK_FALSE;
	depth_stencil_information.front = {};
	depth_stencil_information.back = {};

	static constexpr std::array<VkDynamicState, 2> DYNAMIC_STATES
	{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
	VkPipelineDynamicStateCreateInfo dynamic_state_information{};
	dynamic_state_information.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_information.dynamicStateCount = static_cast<uint32_t>(DYNAMIC_STATES.size());
	dynamic_state_information.pDynamicStates = DYNAMIC_STATES.data();

	VkDescriptorSetLayout set_layouts[]{descriptor_set_layout};
	VkPipelineLayoutCreateInfo pipeline_layout_information{};
	pipeline_layout_information.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_information.setLayoutCount = 1;
	pipeline_layout_information.pSetLayouts = set_layouts;
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
	pipeline_information.pDepthStencilState = &depth_stencil_information;
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
		std::array<VkImageView, 2> attachments{swapchain_image_views[i], depth_image_view};

		VkFramebufferCreateInfo framebuffer_information{};
		framebuffer_information.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_information.renderPass = render_pass;
		framebuffer_information.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_information.pAttachments = attachments.data();
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
		!= VK_SUCCESS) error("Could not create a Vulkan command pool.");
}

void Vulkan::create_image(uint32_t width, uint32_t height, VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	Vulkan_Deleter<VkImage>& texture, Vulkan_Deleter<VkDeviceMemory>& texture_memory)
{
	VkImageCreateInfo image_information{};
	image_information.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_information.imageType = VK_IMAGE_TYPE_2D;
	image_information.extent.width = width;
	image_information.extent.height = height;
	image_information.extent.depth = 1;
	image_information.mipLevels = 1;
	image_information.arrayLayers = 1;
	image_information.format = format;
	image_information.tiling = tiling;
	image_information.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	image_information.usage = usage;
	image_information.samples = VK_SAMPLE_COUNT_1_BIT;
	image_information.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if(vkCreateImage(device, &image_information, nullptr, texture.replace()) != VK_SUCCESS)
		error("Could not create a Vulkan image.");

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, texture, &memory_requirements);
	VkMemoryAllocateInfo allocation_information{};
	allocation_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation_information.allocationSize = memory_requirements.size;
	allocation_information.memoryTypeIndex =
		find_memory(memory_requirements.memoryTypeBits, properties);
	if(vkAllocateMemory(device, &allocation_information, nullptr, texture_memory.replace())
		!= VK_SUCCESS) error("Could not allocate Vulkan image memory.");
	vkBindImageMemory(device, texture, texture_memory, 0);
}

void Vulkan::create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect,
	Vulkan_Deleter<VkImageView>& image_view)
{
	VkImageViewCreateInfo image_view_information{};
	image_view_information.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_information.image = image;
	image_view_information.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_information.format = format;
	image_view_information.subresourceRange.aspectMask = aspect;
	image_view_information.subresourceRange.baseMipLevel = 0;
	image_view_information.subresourceRange.levelCount = 1;
	image_view_information.subresourceRange.baseArrayLayer = 0;
	image_view_information.subresourceRange.layerCount = 1;

	if(vkCreateImageView(device, &image_view_information, nullptr, image_view.replace())
		!= VK_SUCCESS) error("Could not create a Vulkan image view.");
}

VkCommandBuffer Vulkan::begin_single_time_commands()
{
	VkCommandBuffer command_buffer;
	VkCommandBufferAllocateInfo command_buffer_allocation_information{};
	command_buffer_allocation_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocation_information.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocation_information.commandPool = command_pool;
	command_buffer_allocation_information.commandBufferCount = 1;
	vkAllocateCommandBuffers(device, &command_buffer_allocation_information, &command_buffer);

	VkCommandBufferBeginInfo command_buffer_begin_information{};
	command_buffer_begin_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_information);

	return command_buffer;
}

void Vulkan::end_single_time_commands(VkCommandBuffer command_buffer)
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_information{};
	submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_information.commandBufferCount = 1;
	submit_information.pCommandBuffers = &command_buffer;

	vkQueueSubmit(graphics_queue, 1, &submit_information, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void Vulkan::transition_image_layout(VkImage texture, VkFormat format,
	VkImageLayout old_layout, VkImageLayout new_layout)
{
	VkCommandBuffer command_buffer{begin_single_time_commands()};

	VkImageMemoryBarrier texture_memory_barrier{};
	texture_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	texture_memory_barrier.oldLayout = old_layout;
	texture_memory_barrier.newLayout = new_layout;
	texture_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	texture_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	texture_memory_barrier.image = texture;
	texture_memory_barrier.subresourceRange.baseMipLevel = 0;
	texture_memory_barrier.subresourceRange.levelCount = 1;
	texture_memory_barrier.subresourceRange.baseArrayLayer = 0;
	texture_memory_barrier.subresourceRange.layerCount = 1;

	if(new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		texture_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if(format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
			texture_memory_barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else texture_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	if(old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
		new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		texture_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
		texture_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	else if(old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
		new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		texture_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
		texture_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		texture_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		texture_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
		new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		texture_memory_barrier.srcAccessMask = NULL,
		texture_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	else error("Could not complete a Vulkan texture layout transition.");

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &texture_memory_barrier);

	end_single_time_commands(command_buffer);
}

void Vulkan::create_depth_resources()
{
	create_image(swapchain_extent.width, swapchain_extent.height, DEPTH_FORMAT,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);
	create_image_view(depth_image, DEPTH_FORMAT,
		VK_IMAGE_ASPECT_DEPTH_BIT, depth_image_view);
	transition_image_layout(depth_image, DEPTH_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Vulkan::copy_texture(VkImage source_image, VkImage destination_image,
	uint32_t width, uint32_t height)
{
	VkCommandBuffer command_buffer{begin_single_time_commands()};

	VkImageSubresourceLayers texture_subresource_layers{};
	texture_subresource_layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	texture_subresource_layers.baseArrayLayer = 0;
	texture_subresource_layers.mipLevel = 0;
	texture_subresource_layers.layerCount = 1;

	VkImageCopy image_copy{};
	image_copy.srcSubresource = texture_subresource_layers;
	image_copy.dstSubresource = texture_subresource_layers;
	image_copy.srcOffset = {0, 0, 0};
	image_copy.dstOffset = {0, 0, 0};
	image_copy.extent.width = width;
	image_copy.extent.height = height;
	image_copy.extent.depth = 1;

	vkCmdCopyImage(command_buffer, source_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		destination_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);

	end_single_time_commands(command_buffer);
}

void Vulkan::create_texture(const std::string& path)
{
	int width, height, channels;
	stbi_uc *data{stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha)};
	VkDeviceSize size{static_cast<VkDeviceSize>(width*height*4)};
	if(!data) error("Could not load the image \""+path+"\".");

	Vulkan_Deleter<VkImage> staging{device, vkDestroyImage};
	Vulkan_Deleter<VkDeviceMemory> staging_memory{device, vkFreeMemory};
	create_image(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, staging_memory);

	void *memory;
	vkMapMemory(device, staging_memory, 0, size, 0, &memory);
	memcpy(memory, data, (size_t)size);
	vkUnmapMemory(device, staging_memory);
	stbi_image_free(data);

	create_image(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture, texture_memory);

	transition_image_layout(staging, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	transition_image_layout(texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_texture(staging, texture, width, height);
	transition_image_layout(texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Vulkan::create_texture_view()
{ create_image_view(texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, texture_view); }

void Vulkan::create_texture_sampler()
{
	VkSamplerCreateInfo sampler_information{};
	sampler_information.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_information.magFilter = VK_FILTER_LINEAR;
	sampler_information.minFilter = VK_FILTER_LINEAR;
	sampler_information.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_information.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_information.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_information.anisotropyEnable = VK_TRUE;
	sampler_information.maxAnisotropy = 16;
	sampler_information.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_information.unnormalizedCoordinates = VK_FALSE;
	sampler_information.compareEnable = VK_FALSE;
	sampler_information.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_information.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if(vkCreateSampler(device, &sampler_information, nullptr, texture_sampler.replace())
		!= VK_SUCCESS) error("Could not create a Vulkan texture sampler.");
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

void Vulkan::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, Vulkan_Deleter<VkBuffer>& buffer,
	Vulkan_Deleter<VkDeviceMemory>& buffer_memory)
{
	VkBufferCreateInfo buffer_information{};
	buffer_information.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_information.size = size;
	buffer_information.usage = usage;
	buffer_information.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if(vkCreateBuffer(device, &buffer_information, nullptr, buffer.replace()) != VK_SUCCESS)
		error("Could not a create Vulkan buffer.");

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

	VkMemoryAllocateInfo allocation_information{};
	allocation_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation_information.allocationSize = memory_requirements.size;
	allocation_information.memoryTypeIndex =
		find_memory(memory_requirements.memoryTypeBits, properties);
	if(vkAllocateMemory(device, &allocation_information, nullptr,
		buffer_memory.replace()) != VK_SUCCESS) error("Could not allocate Vulkan buffer memory.");

	vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

void Vulkan::copy_buffer(VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size)
{
	VkCommandBuffer command_buffer{begin_single_time_commands()};
	VkBufferCopy copy_region{};
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, 1, &copy_region);
	end_single_time_commands(command_buffer);
}

void Vulkan::create_vertex_buffer()
{
	VkDeviceSize buffer_size{sizeof(vertices[0])*vertices.size()};

	Vulkan_Deleter<VkBuffer> staging_buffer{device, vkDestroyBuffer};
	Vulkan_Deleter<VkDeviceMemory> staging_buffer_memory{device, vkFreeMemory};
	create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);
	void *data;
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, vertices.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer_memory);

	create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);
	copy_buffer(staging_buffer, vertex_buffer, buffer_size);
}

void Vulkan::create_index_buffer()
{
	VkDeviceSize buffer_size{sizeof(indices[0])*indices.size()};
	Vulkan_Deleter<VkBuffer> staging_buffer{device, vkDestroyBuffer};
	Vulkan_Deleter<VkDeviceMemory> staging_buffer_memory{device, vkFreeMemory};
	create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer, staging_buffer_memory);

	void *data;
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, indices.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer_memory);

	create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);
	copy_buffer(staging_buffer, index_buffer, buffer_size);
}

void Vulkan::create_uniform_buffer()
{
	create_buffer(uniform_buffer_object_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		uniform_staging_buffer, uniform_staging_buffer_memory);
	create_buffer(uniform_buffer_object_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		uniform_buffer, uniform_buffer_memory);
}

void Vulkan::create_descriptor_pool()
{
	std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};
	descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_pool_sizes[0].descriptorCount = 1;
	descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_pool_sizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo pool_information{};
	pool_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_information.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
	pool_information.pPoolSizes = descriptor_pool_sizes.data();
	pool_information.maxSets = 1;

	if(vkCreateDescriptorPool(device, &pool_information, nullptr, descriptor_pool.replace())
		!= VK_SUCCESS) error("Could not create a Vulkan descriptor pool.");
}

void Vulkan::create_descriptor_set()
{
	VkDescriptorSetLayout descriptor_set_layouts[]{descriptor_set_layout};
	VkDescriptorSetAllocateInfo allocation_information{};
	allocation_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_information.descriptorPool = descriptor_pool;
	allocation_information.descriptorSetCount = 1;
	allocation_information.pSetLayouts = descriptor_set_layouts;

	if(vkAllocateDescriptorSets(device, &allocation_information, &descriptor_set) != VK_SUCCESS)
		error("Could not allocate a Vulkan descriptor set.");

	VkDescriptorBufferInfo descriptor_buffer_information{};
	descriptor_buffer_information.buffer = uniform_buffer;
	descriptor_buffer_information.offset = 0;
	descriptor_buffer_information.range = uniform_buffer_object_size;

	VkDescriptorImageInfo image_information{};
	image_information.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_information.imageView = texture_view;
	image_information.sampler = texture_sampler;

	std::array<VkWriteDescriptorSet, 2> descriptor_set_writes{};
	descriptor_set_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_set_writes[0].dstSet = descriptor_set;
	descriptor_set_writes[0].dstBinding = 0;
	descriptor_set_writes[0].dstArrayElement = 0;
	descriptor_set_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_set_writes[0].descriptorCount = 1;
	descriptor_set_writes[0].pBufferInfo = &descriptor_buffer_information;

	descriptor_set_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_set_writes[1].dstSet = descriptor_set;
	descriptor_set_writes[1].dstBinding = 1;
	descriptor_set_writes[1].dstArrayElement = 0;
	descriptor_set_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_set_writes[1].descriptorCount = 1;
	descriptor_set_writes[1].pImageInfo = &image_information;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_set_writes.size()),
		descriptor_set_writes.data(), 0, nullptr);
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

		std::array<VkClearValue, 2> clear_colors;
		clear_colors[0] = {0.0f, 0.0f, 0.0f, 1.0f};
		clear_colors[1] = {1.0f, 0};
		VkRenderPassBeginInfo render_pass_information{};
		render_pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_information.renderPass = render_pass;
		render_pass_information.framebuffer = framebuffers[i];
		render_pass_information.renderArea.offset = {0, 0};
		render_pass_information.renderArea.extent = swapchain_extent;
		render_pass_information.clearValueCount = static_cast<uint32_t>(clear_colors.size());
		render_pass_information.pClearValues = clear_colors.data();
		vkCmdBeginRenderPass(command_buffers[i], &render_pass_information,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkBuffer vertex_buffers[]{vertex_buffer};
		VkDeviceSize offsets[]{0};
		vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
		vkCmdBindIndexBuffer(command_buffers[i], index_buffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
		vkCmdDrawIndexed(command_buffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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
			&render_finished_semaphore) != VK_SUCCESS)
		error("Could not create a Vulkan semaphore.");
}

void Vulkan::recreate_swapchain()
{
	vkDeviceWaitIdle(device);
	create_swapchain();
	create_image_views();
	create_render_pass();
	create_graphics_pipeline();
	create_depth_resources();
	create_framebuffers();
	create_command_buffers();
}

void Vulkan::update_uniform_buffer()
{
	void* data;
	vkMapMemory(device, uniform_staging_buffer_memory, 0, uniform_buffer_object_size, 0, &data);
	memcpy(data, uniform_buffer_object, uniform_buffer_object_size);
	vkUnmapMemory(device, uniform_staging_buffer_memory);

	copy_buffer(uniform_staging_buffer, uniform_buffer, uniform_buffer_object_size);
}