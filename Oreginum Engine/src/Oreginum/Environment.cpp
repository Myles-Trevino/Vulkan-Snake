#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "Renderer.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Environment.hpp"

Oreginum::Vulkan::Buffer Oreginum::Environment::vertex_buffer;
Oreginum::Vulkan::Buffer Oreginum::Environment::index_buffer;

Oreginum::Environment::Environment(const std::string& path)
	: texture({path+"/1.hdr", path+"/2.hdr", path+"/3.hdr", path+"/4.hdr",
		path+"/5.hdr", path+"/6.hdr"}, Texture::Format::SRGB, true)
{
	sampler = {Renderer::get_device()};
	if(vertex_buffer.get()) return;
	vertex_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eVertexBuffer, sizeof(float)*VERTICES.size(), VERTICES.data()};
	index_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eIndexBuffer, sizeof(uint16_t)*INDICES.size(), INDICES.data()};
}

void Oreginum::Environment::initialize_descriptor()
{
	descriptor_set = {Renderer::get_device(), Renderer::get_descriptor_pool(),
	{{vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex},
	{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}};

	vk::DescriptorBufferInfo buffer_information
	{Renderer::get_uniform_buffer().get(), 0, sizeof(Renderable::Uniforms)};
	vk::DescriptorImageInfo image_information
	{sampler.get(), texture.get_image().get_view(), vk::ImageLayout::eShaderReadOnlyOptimal};

	descriptor_set.write({{vk::DescriptorType::eUniformBufferDynamic, &buffer_information,
		nullptr}, {vk::DescriptorType::eCombinedImageSampler, nullptr, &image_information}});
}

void Oreginum::Environment::update()
{
	uniforms.projection = Camera::get_projection();
	uniforms.view = Camera::get_view();
	//uniforms.model = glm::translate(translation)*rotation*glm::rotate(
	//	glm::fmat4{}, glm::radians(90.0f), {1, 0, 0})*glm::scale(scale);

	uniforms.camera = Camera::get_position();
}

void Oreginum::Environment::draw(const Vulkan::Command_Buffer& command_buffer,
	uint32_t descriptor_offset)
{
	command_buffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics,
		Renderer::get_environment_pipeline().get());
	command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});
	command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint16);
	command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		Renderer::get_environment_pipeline().get_layout(), 0,
		{descriptor_set.get()}, {descriptor_offset});
	command_buffer.get().drawIndexed(static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);
}