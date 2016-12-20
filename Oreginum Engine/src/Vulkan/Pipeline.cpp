#include <array>
#include "../Oreginum//Core.hpp"
#include "Pipeline.hpp"

Oreginum::Vulkan::Pipeline::Pipeline(const Device& device, const Swapchain& swapchain,
	const Render_Pass& render_pass, const Shader& shader,
	const Vulkan::Descriptor_Set& descriptor_set) : device(&device)
{
	//Vertex input
	std::array<vk::VertexInputBindingDescription, 1> binding_descriptions;
	binding_descriptions[0].setBinding(0);
	//binding_descriptions[0].setStride(sizeof(float)*2);
	binding_descriptions[0].setStride(sizeof(float)*3);
	binding_descriptions[0].setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 1> attribute_descriptions;
	attribute_descriptions[0].setBinding(0);
	attribute_descriptions[0].setLocation(0);
	attribute_descriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
	attribute_descriptions[0].setOffset(0);

	vk::PipelineVertexInputStateCreateInfo vertex_input_state_information;
	vertex_input_state_information.setVertexBindingDescriptionCount(
		static_cast<uint32_t>(binding_descriptions.size()));
	vertex_input_state_information.setPVertexBindingDescriptions(binding_descriptions.data());
	vertex_input_state_information.setVertexAttributeDescriptionCount(
		static_cast<uint32_t>(attribute_descriptions.size()));
	vertex_input_state_information.setPVertexAttributeDescriptions(
		attribute_descriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_information;
	input_assembly_state_information.setTopology(vk::PrimitiveTopology::eTriangleList);
	input_assembly_state_information.setPrimitiveRestartEnable(VK_FALSE);

	//Viewport
	vk::Viewport viewport;
	viewport.setX(0);
	viewport.setY(0);
	viewport.setWidth(static_cast<float>(swapchain.get_extent().width));
	viewport.setHeight(static_cast<float>(swapchain.get_extent().height));
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	vk::Rect2D scissor;
	scissor.setOffset({0, 0});
	scissor.setExtent(swapchain.get_extent());

	vk::PipelineViewportStateCreateInfo viewport_state_information;
	viewport_state_information.setViewportCount(1);
	viewport_state_information.setPViewports(&viewport);
	viewport_state_information.setScissorCount(1);
	viewport_state_information.setPScissors(&scissor);

	//Rasterization
	vk::PipelineRasterizationStateCreateInfo rasterization_state_information;
	rasterization_state_information.setDepthClampEnable(VK_FALSE);
	rasterization_state_information.setRasterizerDiscardEnable(VK_FALSE);
	rasterization_state_information.setPolygonMode(vk::PolygonMode::eFill);
	rasterization_state_information.setCullMode(vk::CullModeFlagBits::eNone);
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

	//Blending
	vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
	color_blend_attachment_state.setBlendEnable(VK_FALSE);
	color_blend_attachment_state.setColorWriteMask(vk::ColorComponentFlagBits::eR | 
		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | 
		vk::ColorComponentFlagBits::eA);

	vk::PipelineColorBlendStateCreateInfo color_blend_state_information;
	color_blend_state_information.setLogicOpEnable(VK_FALSE);
	color_blend_state_information.setLogicOp(vk::LogicOp::eCopy);
	color_blend_state_information.setAttachmentCount(1);
	color_blend_state_information.setPAttachments(&color_blend_attachment_state);
	color_blend_state_information.setBlendConstants({0, 0, 0, 0});

	//Layout
	vk::PipelineLayoutCreateInfo layout_information;
	layout_information.setSetLayoutCount(1);
	layout_information.setPSetLayouts(&descriptor_set.get_layout());
	layout_information.setPushConstantRangeCount(0);
	layout_information.setPPushConstantRanges(nullptr);

	device.get().waitIdle();
	if(device.get().createPipelineLayout(&layout_information,
		nullptr, &pipeline_layout) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline layout.");

	//Pipeline
	vk::GraphicsPipelineCreateInfo pipeline_information;
	pipeline_information.setStageCount(static_cast<uint32_t>(shader.get().size()));
	pipeline_information.setPStages(shader.get().data());
	pipeline_information.setPVertexInputState(&vertex_input_state_information);
	pipeline_information.setPInputAssemblyState(&input_assembly_state_information);
	pipeline_information.setPTessellationState(nullptr);
	pipeline_information.setPViewportState(&viewport_state_information);
	pipeline_information.setPRasterizationState(&rasterization_state_information);
	pipeline_information.setPMultisampleState(&multisample_state_information);
	pipeline_information.setPDepthStencilState(nullptr);
	pipeline_information.setPColorBlendState(&color_blend_state_information);
	pipeline_information.setPDynamicState(nullptr);
	pipeline_information.setLayout(pipeline_layout);
	pipeline_information.setRenderPass(render_pass.get());
	pipeline_information.setSubpass(0);
	pipeline_information.setBasePipelineHandle(VK_NULL_HANDLE);
	pipeline_information.setBasePipelineIndex(-1);

	if(device.get().createGraphicsPipelines(VK_NULL_HANDLE, 1,
		&pipeline_information, nullptr, pipeline.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan graphics pipeline.");
}

Oreginum::Vulkan::Pipeline::~Pipeline()
{
	if(!pipeline.unique() || !device) return;
	device->get().waitIdle();

	if(*pipeline) device->get().destroyPipeline(*pipeline);
	if(pipeline_layout) device->get().destroyPipelineLayout(pipeline_layout);
}

void Oreginum::Vulkan::Pipeline::swap(Pipeline *other)
{
	std::swap(this->device, other->device);
	std::swap(this->pipeline_layout, other->pipeline_layout);
	std::swap(this->pipeline, other->pipeline);
}