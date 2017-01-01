#include "Renderer.hpp"
#include "Rectangle.hpp"

Oreginum::Vulkan::Buffer Oreginum::Rectangle::vertex_buffer;
Oreginum::Vulkan::Buffer Oreginum::Rectangle::index_buffer;

Oreginum::Rectangle::Rectangle(const glm::fvec2& translation, const glm::fvec2& scale,
	const glm::fvec3& color) : scale(scale), translation(translation)
{
	uniforms.color = color;
	if(vertex_buffer.get()) return;
	vertex_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eVertexBuffer, sizeof(float)*VERTICES.size(), VERTICES.data()};
	index_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eIndexBuffer, sizeof(uint16_t)*INDICES.size(), INDICES.data()};
}

void Oreginum::Rectangle::update()
{
	uniforms.matrix = glm::ortho(0.0f, (float)Window::get_resolution().x,
		0.0f, (float)Window::get_resolution().y, -1.0f, 1.0f)*
		glm::translate(glm::fvec3{translation, 0})*glm::scale(glm::fvec3{scale, 0});
}

void Oreginum::Rectangle::draw(const Vulkan::Descriptor_Set& descriptor_set,
	const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset)
{
	command_buffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics,
		Renderer::get_primitive_2d_pipeline().get());
	command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});
	command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint16);
	command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		Renderer::get_primitive_2d_pipeline().get_layout(),
		0, {descriptor_set.get()}, {descriptor_offset});
	command_buffer.get().drawIndexed(static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);
}