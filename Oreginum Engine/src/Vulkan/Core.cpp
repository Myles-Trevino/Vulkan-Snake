#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <map>
#include <set>
#include <array>
#define STB_IMAGE_IMPLEMENTATION
#include <STB Image/stb_image.h>
#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "Core.hpp"

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

	bool debug;
	const Oreginum::Model *model;
	const void *uniform_buffer_object;
	size_t uniform_buffer_object_size;

	Oreginum::Vulkan::Instance instance;
	Oreginum::Vulkan::Device device;

	VkExtent2D swapchain_extent;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchain_images;
	VkCommandPool command_pool;

	VkRenderPass render_pass;
	std::vector<VkImageView> swapchain_image_views;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;

	Oreginum::Vulkan::Buffer vertex_buffer;
	Oreginum::Vulkan::Buffer index_buffer;
	Oreginum::Vulkan::Buffer uniform_buffer;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	std::array<Virtual_Frame, 3> virtual_frames;

	void create_semaphore(VkSemaphore *semaphore)
	{
		VkSemaphoreCreateInfo semaphore_information;
		semaphore_information.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_information.pNext = nullptr;
		semaphore_information.flags = NULL;

		if(vkCreateSemaphore(device.get(), &semaphore_information, nullptr, semaphore) != 
			VK_SUCCESS) Oreginum::Core::error("Could not create Vulkan semaphores.");
	}

	void create_fence(VkFence *fence, VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT)
	{
		VkFenceCreateInfo fence_information;
		fence_information.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_information.pNext = nullptr;
		fence_information.flags = flags;

		if(vkCreateFence(device.get(), &fence_information, nullptr, fence) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan fence.");
	}

	void create_image_view(VkImageView *image_view, VkImage image, VkFormat format,
		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
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
		image_view_information.subresourceRange = {aspect, 0, 1, 0, 1};

		if(vkCreateImageView(device.get(), &image_view_information, nullptr, image_view) != 
			VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan image view.");
	}

	void create_swapchain()
	{
		device.update();
		swapchain_extent = device.get_surface_capabilities().currentExtent;
		VkSwapchainKHR old_swapchain{swapchain};

		VkSwapchainCreateInfoKHR swapchain_information;
		swapchain_information.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_information.pNext = nullptr;
		swapchain_information.flags = NULL;
		swapchain_information.surface = instance.get_surface();
		swapchain_information.minImageCount =
			std::max(Oreginum::Vulkan::Core::SWAPCHAIN_MINIMUM_IMAGE_COUNT, 
				device.get_surface_capabilities().minImageCount);
		swapchain_information.imageFormat = Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.format;
		swapchain_information.imageColorSpace = Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.colorSpace;
		swapchain_information.imageExtent = swapchain_extent;
		swapchain_information.imageArrayLayers = 1;
		swapchain_information.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_information.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_information.queueFamilyIndexCount = NULL;
		swapchain_information.pQueueFamilyIndices = nullptr;
		swapchain_information.preTransform = device.get_surface_capabilities().currentTransform;
		swapchain_information.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_information.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchain_information.clipped = VK_TRUE;
		swapchain_information.oldSwapchain = old_swapchain;

		if(vkCreateSwapchainKHR(device.get(), &swapchain_information, nullptr, &swapchain) !=
			VK_SUCCESS) Oreginum::Core::error("Could not create Vulkan swapchain.");
		if(old_swapchain) vkDestroySwapchainKHR(device.get(), old_swapchain, nullptr);

		uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(device.get(), swapchain, &swapchain_image_count, nullptr);
		swapchain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(device.get(), swapchain, &swapchain_image_count, swapchain_images.data());

		for(VkImageView& i : swapchain_image_views) vkDestroyImageView(device.get(), i, nullptr);
		swapchain_image_views.resize(swapchain_images.size());
		for(size_t i{}; i < swapchain_image_views.size(); ++i)
			create_image_view(&swapchain_image_views[i], swapchain_images[i], 
			Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.format);
	}

	void create_render_pass()
	{
		//Attachments
		VkAttachmentDescription color_attachment_description;
		color_attachment_description.flags = NULL;
		color_attachment_description.format = Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.format;
		color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth_attachment_description;
		depth_attachment_description.flags = NULL;
		depth_attachment_description.format = Oreginum::Vulkan::Core::DEPTH_FORMAT;
		depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_description.initialLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//Subpasses
		VkAttachmentReference color_attachment_reference;
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_reference;
		depth_attachment_reference.attachment = 1;
		depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description;
		subpass_description.flags = NULL;
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.inputAttachmentCount = 0;
		subpass_description.pInputAttachments = nullptr;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attachment_reference;
		subpass_description.pResolveAttachments = nullptr;
		subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
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
		std::array<VkAttachmentDescription, 2> attachments
		{color_attachment_description, depth_attachment_description};
		VkRenderPassCreateInfo render_pass_information;
		render_pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_information.pNext = nullptr;
		render_pass_information.flags = NULL;
		render_pass_information.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_information.pAttachments = attachments.data();
		render_pass_information.subpassCount = 1;
		render_pass_information.pSubpasses = &subpass_description;
		render_pass_information.dependencyCount = static_cast<uint32_t>(dependencies.size());
		render_pass_information.pDependencies = dependencies.data();

		vkCreateRenderPass(device.get(), &render_pass_information, nullptr, &render_pass);
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
		if(vkCreateShaderModule(device.get(), &shader_module_information,
			nullptr, &shader_module) != VK_SUCCESS)
			Oreginum::Core::error("Could not create Vulkan shader module \""+shader+"\".");

		return shader_module;
	}

	void create_descriptor_set_layout(VkDescriptorSetLayout *descriptor_set_layout,
		uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage_flags,
		uint32_t count = 1, VkSampler *immutable_samplers = nullptr)
	{
		VkDescriptorSetLayoutBinding descriptor_set_layout_binding;
		descriptor_set_layout_binding.binding = binding;
		descriptor_set_layout_binding.descriptorType = type;
		descriptor_set_layout_binding.descriptorCount = count;
		descriptor_set_layout_binding.stageFlags = stage_flags;
		descriptor_set_layout_binding.pImmutableSamplers = immutable_samplers;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_information;
		descriptor_set_layout_information.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_information.pNext = nullptr;
		descriptor_set_layout_information.flags = NULL;
		descriptor_set_layout_information.bindingCount = 1;
		descriptor_set_layout_information.pBindings = &descriptor_set_layout_binding;

		if(vkCreateDescriptorSetLayout(device.get(), &descriptor_set_layout_information,
			nullptr, descriptor_set_layout) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan descriptor set layout.");
	}

	void create_graphics_pipeline()
	{
		//Shaders
		VkShaderModule vertex_shader_module{create_shader_module("Basic Vertex")};
		VkShaderModule fragment_shader_module{create_shader_module("Basic Fragment")};

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
		vertex_input_binding_description.stride = sizeof(float)*8;
		vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 3> vertex_attribute_descriptions;
		vertex_attribute_descriptions[0].location = 0;
		vertex_attribute_descriptions[0].binding = vertex_input_binding_description.binding;
		vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_descriptions[0].offset = 0;
		vertex_attribute_descriptions[1].location = 1;
		vertex_attribute_descriptions[1].binding = vertex_input_binding_description.binding;
		vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_descriptions[1].offset = sizeof(float)*3;
		vertex_attribute_descriptions[2].location = 2;
		vertex_attribute_descriptions[2].binding = vertex_input_binding_description.binding;
		vertex_attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertex_attribute_descriptions[2].offset = sizeof(float)*6;

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
		input_assembly_state_information.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

		//Depth stencil
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state_information;
		depth_stencil_state_information.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_information.pNext = nullptr;
		depth_stencil_state_information.flags = NULL;
		depth_stencil_state_information.depthTestEnable = VK_TRUE;
		depth_stencil_state_information.depthWriteEnable = VK_TRUE;
		depth_stencil_state_information.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state_information.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_information.stencilTestEnable = VK_FALSE;
		depth_stencil_state_information.front = {};
		depth_stencil_state_information.back = {};
		depth_stencil_state_information.minDepthBounds = 0;
		depth_stencil_state_information.maxDepthBounds = 1;

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
		layout_information.setLayoutCount = 1;
		layout_information.pSetLayouts = &descriptor_set_layout;
		layout_information.pushConstantRangeCount = 0;
		layout_information.pPushConstantRanges = nullptr;

		if(vkCreatePipelineLayout(device.get(), &layout_information, nullptr, &pipeline_layout) !=
			VK_SUCCESS)
		{
			vkDestroyShaderModule(device.get(), fragment_shader_module, nullptr);
			vkDestroyShaderModule(device.get(), vertex_shader_module, nullptr);
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
		pipeline_information.pDepthStencilState = &depth_stencil_state_information;
		pipeline_information.pColorBlendState = &color_blend_state_information;
		pipeline_information.pDynamicState = &dynamic_state_create_info;
		pipeline_information.layout = pipeline_layout;
		pipeline_information.renderPass = render_pass;
		pipeline_information.subpass = 0;
		pipeline_information.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_information.basePipelineIndex = -1;

		if(vkCreateGraphicsPipelines(device.get(), VK_NULL_HANDLE, 1,
			&pipeline_information, nullptr, &pipeline) != VK_SUCCESS)
		{
			vkDestroyShaderModule(device.get(), fragment_shader_module, nullptr);
			vkDestroyShaderModule(device.get(), vertex_shader_module, nullptr);
			Oreginum::Core::error("Could not create a Vulkan graphics pipeline.");
		}

		vkDestroyShaderModule(device.get(), fragment_shader_module, nullptr);
		vkDestroyShaderModule(device.get(), vertex_shader_module, nullptr);
	}

	void copy_data_old(VkCommandBuffer command_buffer, VkBuffer staging_buffer,
		VkBuffer device_buffer, VkDeviceMemory staging_memory, const void *data, size_t data_size)
	{
		//Copy data to staging buffer
		void *memory_pointer;
		if(vkMapMemory(device.get(), staging_memory, 0, data_size, NULL, &memory_pointer))
			Oreginum::Core::error("Could not map Vulkan staging buffer memory.");

		memcpy(memory_pointer, data, data_size);

		VkMappedMemoryRange mapped_memory_range;
		mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_memory_range.pNext = nullptr;
		mapped_memory_range.memory = staging_memory;
		mapped_memory_range.offset = 0;
		mapped_memory_range.size = data_size;

		vkFlushMappedMemoryRanges(device.get(), 1, &mapped_memory_range);
		vkUnmapMemory(device.get(), staging_memory);

		//Copy data to device buffer
		VkCommandBufferBeginInfo command_buffer_information;
		command_buffer_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_information.pNext = nullptr;
		command_buffer_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_information.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(command_buffer, &command_buffer_information);

		VkBufferCopy buffer_copy{0, 0, data_size};
		vkCmdCopyBuffer(command_buffer, staging_buffer, device_buffer, 1, &buffer_copy);

		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_information;
		submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_information.pNext = nullptr;
		submit_information.waitSemaphoreCount = 0;
		submit_information.pWaitSemaphores = nullptr;
		submit_information.pWaitDstStageMask = nullptr;
		submit_information.commandBufferCount = 1;
		submit_information.pCommandBuffers = &command_buffer;
		submit_information.signalSemaphoreCount = 0;
		submit_information.pSignalSemaphores = nullptr;

		if(vkQueueSubmit(device.get_graphics_queue(), 1, &submit_information, VK_NULL_HANDLE) != 
			VK_SUCCESS) Oreginum::Core::error("Could not submit a Vulkan command buffer.");

		vkDeviceWaitIdle(device.get());
	}

	void create_command_pool(VkCommandPool *command_pool, VkCommandPoolCreateFlags flags,
		uint32_t queue_family_index = device.get_graphics_queue_family_index())
	{
		VkCommandPoolCreateInfo pool_information;
		pool_information.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_information.pNext = nullptr;
		pool_information.flags = flags;
		pool_information.queueFamilyIndex = queue_family_index;

		if(vkCreateCommandPool(device.get(), &pool_information, nullptr, command_pool) != 
			VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan command pool.");
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

		if(vkAllocateCommandBuffers(device.get(), &command_buffer_information, command_buffers) !=
			VK_SUCCESS) Oreginum::Core::error("Could not allocate a Vulkan command buffer.");
	}

	void create_descriptor_pool(VkDescriptorType type,
		uint32_t count = 1, uint32_t maximum_sets = 1)
	{
		VkDescriptorPoolSize descriptor_pool_size;
		descriptor_pool_size.type = type;
		descriptor_pool_size.descriptorCount = count;

		VkDescriptorPoolCreateInfo descriptor_pool_information;
		descriptor_pool_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_information.pNext = nullptr;
		descriptor_pool_information.flags = NULL;
		descriptor_pool_information.maxSets = maximum_sets;
		descriptor_pool_information.poolSizeCount = 1;
		descriptor_pool_information.pPoolSizes = &descriptor_pool_size;

		if(vkCreateDescriptorPool(device.get(), &descriptor_pool_information,
			nullptr, &descriptor_pool) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan descriptor pool.");
	}

	void create_descriptor_set(VkDescriptorSet *descriptor_sets, VkDescriptorPool pool,
		std::vector<VkDescriptorSetLayout> layouts)
	{
		VkDescriptorSetAllocateInfo descriptor_set_allocate_information;
		descriptor_set_allocate_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_allocate_information.pNext = nullptr;
		descriptor_set_allocate_information.descriptorPool = pool;
		descriptor_set_allocate_information.descriptorSetCount =
			static_cast<uint32_t>(layouts.size());
		descriptor_set_allocate_information.pSetLayouts = layouts.data();

		if(vkAllocateDescriptorSets(device.get(), &descriptor_set_allocate_information,
			descriptor_sets) != VK_SUCCESS)
			Oreginum::Core::error("Could not allocate a Vulkan descriptor set.");
	}

	void write_buffer_descriptor_set(VkBuffer buffer, VkDeviceSize range,
		VkDescriptorSet descriptor_set, uint32_t binding, VkDescriptorType descriptor_type,
		uint32_t descriptor_count = 1, uint32_t element = 0, VkDeviceSize offset = 0)
	{
		VkDescriptorBufferInfo descriptor_buffer_information;
		descriptor_buffer_information.buffer = buffer;
		descriptor_buffer_information.offset = offset;
		descriptor_buffer_information.range = range;

		VkWriteDescriptorSet write_descriptor_set;
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set.pNext = nullptr;
		write_descriptor_set.dstSet = descriptor_set;
		write_descriptor_set.dstBinding = binding;
		write_descriptor_set.dstArrayElement = element;
		write_descriptor_set.descriptorCount = descriptor_count;
		write_descriptor_set.descriptorType = descriptor_type;
		write_descriptor_set.pImageInfo = nullptr;
		write_descriptor_set.pBufferInfo = &descriptor_buffer_information;
		write_descriptor_set.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device.get(), 1, &write_descriptor_set, 0, nullptr);
	}

	void create_image(VkImage *image, VkDeviceMemory *image_memory, const glm::uvec2& resolution,
		VkFormat format = Oreginum::Vulkan::Core::IMAGE_FORMAT,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		uint32_t mip_levels = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
		VkImageType type = VK_IMAGE_TYPE_2D, uint32_t array_layers = 1,
		VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
		const std::vector<uint32_t>& queue_family_indices = {})
	{
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

		if(vkCreateImage(device.get(), &image_information, nullptr, image) != VK_SUCCESS)
			Oreginum::Core::error("Could not create a Vulkan image.");

		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(device.get(), *image, &memory_requirements);

		VkMemoryAllocateInfo memory_information;
		memory_information.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_information.pNext = nullptr;
		memory_information.allocationSize = memory_requirements.size;
		memory_information.memoryTypeIndex =
			Oreginum::Vulkan::Core::find_memory(memory_requirements.memoryTypeBits, properties);

		if(vkAllocateMemory(device.get(), &memory_information, nullptr, image_memory) != VK_SUCCESS)
			Oreginum::Core::error("Could not allocate memory for a Vulkan image.");

		vkBindImageMemory(device.get(), *image, *image_memory, 0);
	}

	void transition_image_layout(VkImage image, VkFormat format,
		VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags source_access_flags,
		VkAccessFlags destination_access_flags,
		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		uint32_t source_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
		uint32_t destination_family_queue_index = VK_QUEUE_FAMILY_IGNORED)
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

	void create_framebuffer(VkFramebuffer *framebuffer, VkImageView image_view, VkExtent2D extent)
	{
		std::array<VkImageView, 2> attachments{image_view, depth_image_view};

		VkFramebufferCreateInfo framebuffer_information;
		framebuffer_information.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_information.pNext = nullptr;
		framebuffer_information.flags = NULL;
		framebuffer_information.renderPass = render_pass;
		framebuffer_information.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_information.pAttachments = attachments.data();
		framebuffer_information.width = extent.width;
		framebuffer_information.height = extent.height;
		framebuffer_information.layers = 1;

		if(vkCreateFramebuffer(device.get(), &framebuffer_information, nullptr, framebuffer) !=
			VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan framebuffer.");
	}

	void prepare_frame(Virtual_Frame *virtual_frame, VkImageView image_view)
	{
		//Update uniform buffer
		uniform_buffer.fill(uniform_buffer_object, uniform_buffer_object_size);

		if(virtual_frame->framebuffer)
			vkDestroyFramebuffer(device.get(), virtual_frame->framebuffer, nullptr);
		create_framebuffer(&virtual_frame->framebuffer, image_view, swapchain_extent);

		VkCommandBufferBeginInfo command_buffer_information;
		command_buffer_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_information.pNext = nullptr;
		command_buffer_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_information.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(virtual_frame->command_buffer, &command_buffer_information);

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {0, 0, 0, 1};
		clear_values[1].depthStencil = {1, 0};

		VkRenderPassBeginInfo render_pass_begin_information;
		render_pass_begin_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_information.pNext = nullptr;
		render_pass_begin_information.renderPass = render_pass;
		render_pass_begin_information.framebuffer = virtual_frame->framebuffer;
		render_pass_begin_information.renderArea = {{0, 0}, swapchain_extent};
		render_pass_begin_information.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_information.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(virtual_frame->command_buffer, &render_pass_begin_information,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(virtual_frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport{0, 0, static_cast<float>(swapchain_extent.width),
			static_cast<float>(swapchain_extent.height), 0, 1};
		VkRect2D scissor{{0, 0}, swapchain_extent};

		vkCmdSetViewport(virtual_frame->command_buffer, 0, 1, &viewport);
		vkCmdSetScissor(virtual_frame->command_buffer, 0, 1, &scissor);

		VkDeviceSize offsets{0};
		vkCmdBindVertexBuffers(virtual_frame->command_buffer,
			0, 1, vertex_buffer.get(), &offsets);

		vkCmdBindIndexBuffer(virtual_frame->command_buffer,
			*index_buffer.get(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(virtual_frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

		vkCmdDrawIndexed(virtual_frame->command_buffer,
			static_cast<uint32_t>(model->get_indices().size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(virtual_frame->command_buffer);

		if(vkEndCommandBuffer(virtual_frame->command_buffer) != VK_SUCCESS)
			Oreginum::Core::error("Could not record a Vulkan command buffer.");
	}

	void create_swapchain_dependants()
	{
		//Depth stencil
		create_image(&depth_image, &depth_image_memory, {swapchain_extent.width,
			swapchain_extent.height}, Oreginum::Vulkan::Core::DEPTH_FORMAT, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		create_image_view(&depth_image_view, depth_image,
			Oreginum::Vulkan::Core::DEPTH_FORMAT, VK_IMAGE_ASPECT_DEPTH_BIT);
		transition_image_layout(depth_image,
			Oreginum::Vulkan::Core::DEPTH_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, NULL,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void destroy_swapchain_dependants()
	{
		//Depth stencil
		if(depth_image_view) vkDestroyImageView(device.get(), depth_image_view, nullptr);
		if(depth_image_memory) vkFreeMemory(device.get(), depth_image_memory, nullptr);
		if(depth_image) vkDestroyImage(device.get(), depth_image, nullptr);
	}

	void recreate_swapchain()
	{
		if(device.get()) vkDeviceWaitIdle(device.get());
		destroy_swapchain_dependants();
		create_swapchain();
		create_swapchain_dependants();
	}
}

VkCommandBuffer Oreginum::Vulkan::Core::begin_single_time_commands()
{
	VkCommandBufferAllocateInfo command_buffer_allocate_information;
	command_buffer_allocate_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_information.pNext = nullptr;
	command_buffer_allocate_information.commandPool = command_pool;
	command_buffer_allocate_information.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_information.commandBufferCount = 1;

	VkCommandBufferBeginInfo command_buffer_begin_information;
	command_buffer_begin_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_information.pNext = nullptr;
	command_buffer_begin_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	command_buffer_begin_information.pInheritanceInfo = nullptr;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(device.get(), &command_buffer_allocate_information, &command_buffer);

	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_information);

	return command_buffer;
}

void Oreginum::Vulkan::Core::end_single_time_commands(VkCommandBuffer command_buffer)
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_information;
	submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_information.pNext = nullptr;
	submit_information.waitSemaphoreCount = 0;
	submit_information.pWaitSemaphores = nullptr;
	submit_information.commandBufferCount = 1;
	submit_information.pCommandBuffers = &command_buffer;
	submit_information.signalSemaphoreCount = 0;
	submit_information.pSignalSemaphores = nullptr;

	vkQueueSubmit(device.get_graphics_queue(), 1, &submit_information, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.get_graphics_queue());

	vkFreeCommandBuffers(device.get(), command_pool, 1, &command_buffer);
}

void Oreginum::Vulkan::Core::initialize(const Oreginum::Model& model,
	const void *uniform_buffer_object,
	size_t uniform_buffer_object_size, bool debug)
{
	::model = &model;
	::debug = debug;
	::uniform_buffer_object = uniform_buffer_object;
	::uniform_buffer_object_size = uniform_buffer_object_size;

	instance.initialize(debug);
	device.initialize(&instance);
	create_swapchain();

	create_render_pass();
	create_descriptor_set_layout(&descriptor_set_layout, 0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	create_graphics_pipeline();

	create_command_pool(&command_pool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	for(auto& v : virtual_frames)
	{
		allocate_command_buffers(command_pool, &v.command_buffer);
		create_semaphore(&v.swapchain_image_available_semaphore);
		create_semaphore(&v.rendering_finished_semaphore);
		create_fence(&v.fence);
	}

	//Vertex buffer
	vertex_buffer.initialize(&device, model.get_vertices().data(), sizeof(model.get_vertices()[0])*
		model.get_vertices().size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	//Indice buffer
	index_buffer.initialize(&device, model.get_indices().data(), sizeof(model.get_indices()[0])*
		model.get_indices().size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	//Uniform buffer
	uniform_buffer.initialize(&device, nullptr, uniform_buffer_object_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	create_descriptor_pool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	create_descriptor_set(&descriptor_set, descriptor_pool, {descriptor_set_layout});
	write_buffer_descriptor_set(*uniform_buffer.get(), uniform_buffer_object_size,
		descriptor_set, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	create_swapchain_dependants();
}

void Oreginum::Vulkan::Core::destroy()
{
	if(device.get()) vkDeviceWaitIdle(device.get());

	for(Virtual_Frame& v : virtual_frames)
		if(v.framebuffer) vkDestroyFramebuffer(device.get(), v.framebuffer, nullptr);

	destroy_swapchain_dependants();

	if(descriptor_pool) vkDestroyDescriptorPool(device.get(), descriptor_pool, nullptr);

	for(Virtual_Frame& v : virtual_frames)
	{
		if(v.fence) vkDestroyFence(device.get(), v.fence, nullptr);
		if(v.rendering_finished_semaphore)
			vkDestroySemaphore(device.get(), v.rendering_finished_semaphore, nullptr);
		if(v.swapchain_image_available_semaphore)
			vkDestroySemaphore(device.get(), v.swapchain_image_available_semaphore, nullptr);
		if(v.command_buffer) vkFreeCommandBuffers(device.get(), command_pool, 1, &v.command_buffer);
	}
	if(command_pool) vkDestroyCommandPool(device.get(), command_pool, nullptr);

	if(pipeline) vkDestroyPipeline(device.get(), pipeline, nullptr);
	if(pipeline_layout) vkDestroyPipelineLayout(device.get(), pipeline_layout, nullptr);
	if(descriptor_set_layout) vkDestroyDescriptorSetLayout(device.get(), descriptor_set_layout, nullptr);
	if(render_pass) vkDestroyRenderPass(device.get(), render_pass, nullptr);

	for(VkImageView& i : swapchain_image_views) vkDestroyImageView(device.get(), i, nullptr);
	if(swapchain) vkDestroySwapchainKHR(device.get(), swapchain, nullptr);
}

uint32_t Oreginum::Vulkan::Core::find_memory(uint32_t type, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(device.get_gpu(), &memory_properties);

	for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
		if((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties))
			return i;

	Oreginum::Core::error("Could not find suitable Vulkan memory.");
}

void Oreginum::Vulkan::Core::render()
{
	//Recreate swapchain on resize
	if(Window::was_resized()) recreate_swapchain();

	//Virtual frame
	static size_t virtual_frame_index{};
	virtual_frame_index = (virtual_frame_index+1)%virtual_frames.size();
	Virtual_Frame *virtual_frame = &virtual_frames[virtual_frame_index];
	vkWaitForFences(device.get(), 1, &virtual_frame->fence, VK_FALSE, UINT64_MAX);
	vkResetFences(device.get(), 1, &virtual_frame->fence);

	//Swapchain image
	uint32_t image_index;
	VkResult result{vkAcquireNextImageKHR(device.get(), swapchain, UINT64_MAX,
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

	if(vkQueueSubmit(device.get_graphics_queue(), 1, &submit_information, virtual_frame->fence)
		!= VK_SUCCESS) Oreginum::Core::error("Could not submit Vulkan render queue.");

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

	result = vkQueuePresentKHR(device.get_graphics_queue(), &present_information);
	if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) recreate_swapchain();
	else if(result != VK_SUCCESS)
		Oreginum::Core::error("Could not submit Vulkan presentation queue.");
}