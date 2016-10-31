#include <array>
#include "../Oreginum//Core.hpp"
#include "Pipeline.hpp"

void Oreginum::Vulkan::Pipeline::destroy()
{
	if(pipeline) vkDestroyPipeline(device->get(), pipeline, nullptr);
	if(pipeline_layout) vkDestroyPipelineLayout(device->get(), pipeline_layout, nullptr);
}

void Oreginum::Vulkan::Pipeline::initialize(const Device *device, const Shader& shader,
	const Descriptor& descriptor, const Render_Pass& render_pass)
{
	this->device = device;
	destroy();

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
	std::array<VkDescriptorSetLayout, 1> layouts{descriptor.get_layout()};
	VkPipelineLayoutCreateInfo layout_information;
	layout_information.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_information.pNext = nullptr;
	layout_information.flags = NULL;
	layout_information.setLayoutCount = static_cast<uint32_t>(layouts.size());
	layout_information.pSetLayouts = layouts.data();
	layout_information.pushConstantRangeCount = 0;
	layout_information.pPushConstantRanges = nullptr;

	if(vkCreatePipelineLayout(device->get(), &layout_information,
		nullptr, &pipeline_layout) != 	VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline layout.");

	//Pipeline
	VkGraphicsPipelineCreateInfo pipeline_information;
	pipeline_information.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_information.pNext = nullptr;
	pipeline_information.flags = NULL;
	pipeline_information.stageCount = static_cast<uint32_t>(shader.get_information().size());
	pipeline_information.pStages = shader.get_information().data();
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
	pipeline_information.renderPass = render_pass.get();
	pipeline_information.subpass = 0;
	pipeline_information.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_information.basePipelineIndex = -1;

	if(vkCreateGraphicsPipelines(device->get(), VK_NULL_HANDLE, 1,
		&pipeline_information, nullptr, &pipeline) != VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline.");
}