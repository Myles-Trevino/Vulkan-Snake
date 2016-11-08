#include <array>
#include "../Oreginum//Core.hpp"
#include "Pipeline.hpp"

Oreginum::Vulkan::Pipeline::Pipeline(const Device& device, const Shader& shader,
	const Descriptor_Set& descriptor_set, const Render_Pass& render_pass) : device(device)
{
	//Vertex input
	vk::VertexInputBindingDescription vertex_input_binding_description;
	vertex_input_binding_description.setBinding(0);
	vertex_input_binding_description.setStride(sizeof(float)*8);
	vertex_input_binding_description.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 3> vertex_attribute_descriptions;
	vertex_attribute_descriptions[0].setLocation(0);
	vertex_attribute_descriptions[0].setBinding(vertex_input_binding_description.binding);
	vertex_attribute_descriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
	vertex_attribute_descriptions[0].setOffset(0);
	vertex_attribute_descriptions[1].setLocation(1);
	vertex_attribute_descriptions[1].setBinding(vertex_input_binding_description.binding);
	vertex_attribute_descriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
	vertex_attribute_descriptions[1].setOffset(sizeof(float)*3);
	vertex_attribute_descriptions[2].setLocation(2);
	vertex_attribute_descriptions[2].setBinding(vertex_input_binding_description.binding);
	vertex_attribute_descriptions[2].setFormat(vk::Format::eR32G32Sfloat);
	vertex_attribute_descriptions[2].setOffset(sizeof(float)*6);

	vk::PipelineVertexInputStateCreateInfo vertex_input_state_information;
	vertex_input_state_information.setVertexBindingDescriptionCount(1);
	vertex_input_state_information.setPVertexBindingDescriptions(
		&vertex_input_binding_description);
	vertex_input_state_information.setVertexAttributeDescriptionCount(
		static_cast<uint32_t>(vertex_attribute_descriptions.size()));
	vertex_input_state_information.setPVertexAttributeDescriptions(
		vertex_attribute_descriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_information;
	input_assembly_state_information.setTopology(vk::PrimitiveTopology::eTriangleList);
	input_assembly_state_information.setPrimitiveRestartEnable(VK_FALSE);

	//Viewport
	vk::PipelineViewportStateCreateInfo viewport_state_information;
	viewport_state_information.setViewportCount(1);
	viewport_state_information.setPViewports(nullptr);
	viewport_state_information.setScissorCount(1);
	viewport_state_information.setPScissors(nullptr);

	//Rasterization
	vk::PipelineRasterizationStateCreateInfo rasterization_state_information;
	rasterization_state_information.setDepthClampEnable(VK_FALSE);
	rasterization_state_information.setRasterizerDiscardEnable(VK_FALSE);
	rasterization_state_information.setPolygonMode(vk::PolygonMode::eFill);
	rasterization_state_information.setCullMode(vk::CullModeFlagBits::eBack);
	rasterization_state_information.setFrontFace(vk::FrontFace::eCounterClockwise);
	rasterization_state_information.setDepthBiasEnable(VK_FALSE);
	rasterization_state_information.setDepthBiasConstantFactor(0);
	rasterization_state_information.setDepthBiasClamp(0);
	rasterization_state_information.setDepthBiasSlopeFactor(0);
	rasterization_state_information.setLineWidth(1);

	//Multisampling
	vk::PipelineMultisampleStateCreateInfo multisample_state_information;
	multisample_state_information.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	multisample_state_information.setSampleShadingEnable(VK_FALSE);
	multisample_state_information.setMinSampleShading(1);
	multisample_state_information.setPSampleMask(nullptr);
	multisample_state_information.setAlphaToCoverageEnable(VK_FALSE);
	multisample_state_information.setAlphaToOneEnable(VK_FALSE);

	//Depth stencil
	vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_information;
	depth_stencil_state_information.setDepthTestEnable(VK_TRUE);
	depth_stencil_state_information.setDepthWriteEnable(VK_TRUE);
	depth_stencil_state_information.setDepthCompareOp(vk::CompareOp::eLess);
	depth_stencil_state_information.setDepthBoundsTestEnable(VK_FALSE);
	depth_stencil_state_information.setStencilTestEnable(VK_FALSE);
	depth_stencil_state_information.setFront({});
	depth_stencil_state_information.setBack({});
	depth_stencil_state_information.setMinDepthBounds(0);
	depth_stencil_state_information.setMaxDepthBounds(1);

	//Blending
	vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
	color_blend_attachment_state.setBlendEnable(VK_FALSE);
	color_blend_attachment_state.setSrcColorBlendFactor(vk::BlendFactor::eOne);
	color_blend_attachment_state.setDstColorBlendFactor(vk::BlendFactor::eZero);
	color_blend_attachment_state.setColorBlendOp(vk::BlendOp::eAdd);
	color_blend_attachment_state.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	color_blend_attachment_state.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	color_blend_attachment_state.setAlphaBlendOp(vk::BlendOp::eAdd);
	color_blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits{});

	vk::PipelineColorBlendStateCreateInfo color_blend_state_information;
	color_blend_state_information.setLogicOpEnable(VK_FALSE);
	color_blend_state_information.setLogicOp(vk::LogicOp::eCopy);
	color_blend_state_information.setAttachmentCount(1);
	color_blend_state_information.setPAttachments(&color_blend_attachment_state);
	color_blend_state_information.setBlendConstants({0, 0, 0, 0});

	//Dynamic state
	std::array<vk::DynamicState, 2> dynamic_states
	{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

	vk::PipelineDynamicStateCreateInfo dynamic_state_create_info;
	dynamic_state_create_info.setDynamicStateCount(static_cast<uint32_t>(dynamic_states.size()));
	dynamic_state_create_info.setPDynamicStates(dynamic_states.data());

	//Layout
	vk::PipelineLayoutCreateInfo layout_information;
	layout_information.setSetLayoutCount(1);
	layout_information.setPSetLayouts(&descriptor_set.get_layout());
	layout_information.setPushConstantRangeCount(0);
	layout_information.setPPushConstantRanges(nullptr);

	if(device.get().createPipelineLayout(&layout_information,
		nullptr, &pipeline_layout) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline layout.");

	//Pipeline
	vk::GraphicsPipelineCreateInfo pipeline_information;
	pipeline_information.setStageCount(static_cast<uint32_t>(shader.get_information().size()));
	pipeline_information.setPStages(shader.get_information().data());
	pipeline_information.setPVertexInputState(&vertex_input_state_information);
	pipeline_information.setPInputAssemblyState(&input_assembly_state_information);
	pipeline_information.setPTessellationState(nullptr);
	pipeline_information.setPViewportState(&viewport_state_information);
	pipeline_information.setPRasterizationState(&rasterization_state_information);
	pipeline_information.setPMultisampleState(&multisample_state_information);
	pipeline_information.setPDepthStencilState(&depth_stencil_state_information);
	pipeline_information.setPColorBlendState(&color_blend_state_information);
	pipeline_information.setPDynamicState(&dynamic_state_create_info);
	pipeline_information.setLayout(pipeline_layout);
	pipeline_information.setRenderPass(render_pass.get());
	pipeline_information.setSubpass(0);
	pipeline_information.setBasePipelineHandle(VK_NULL_HANDLE);
	pipeline_information.setBasePipelineIndex(-1);

	if(device.get().createGraphicsPipelines(VK_NULL_HANDLE, 1,
		&pipeline_information, nullptr, &pipeline) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline.");
}

Oreginum::Vulkan::Pipeline::~Pipeline()
{
	device.get().destroyPipeline(pipeline);
	device.get().destroyPipelineLayout(pipeline_layout);
}
