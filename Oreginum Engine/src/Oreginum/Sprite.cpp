#include "Renderer.hpp"
#include "Window.hpp"
#include "Sprite.hpp"

Oreginum::Vulkan::Buffer Oreginum::Sprite::vertex_buffer;
Oreginum::Vulkan::Buffer Oreginum::Sprite::index_buffer;

Oreginum::Sprite::Sprite(const std::string& path, const glm::fvec2& translation,
	const glm::fvec2& scale) : scale(scale), translation(translation), texture(path)
{
	sampler = {Renderer::get_device()};
	if(vertex_buffer.get()) return;
	vertex_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eVertexBuffer, sizeof(float)*VERTICES.size(), VERTICES.data()};
	index_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eIndexBuffer, sizeof(uint16_t)*INDICES.size(), INDICES.data()};
}

void Oreginum::Sprite::initialize_descriptor()
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

void Oreginum::Sprite::update()
{
	uniforms.matrix = glm::ortho(0.0f, (float)Window::get_resolution().x,
		0.0f, (float)Window::get_resolution().y, -1.0f, 1.0f)*
		glm::translate(glm::fvec3{translation, 0})*glm::scale(glm::fvec3{scale, 0});
}

void Oreginum::Sprite::draw(const Vulkan::Command_Buffer& command_buffer,
	uint32_t descriptor_offset)
{
	command_buffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics,
		Renderer::get_sprite_pipeline().get());
	command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});
	command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint16);
	command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		Renderer::get_sprite_pipeline().get_layout(), 0,
		{descriptor_set.get()}, {descriptor_offset});
	command_buffer.get().drawIndexed(static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);
}