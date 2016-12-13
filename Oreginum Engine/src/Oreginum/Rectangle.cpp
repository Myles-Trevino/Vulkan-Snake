#include "Rectangle.hpp"

Oreginum::Vulkan::Buffer Oreginum::Rectangle::vertex_buffer;
Oreginum::Vulkan::Buffer Oreginum::Rectangle::index_buffer;

Oreginum::Rectangle::Rectangle(const Vulkan::Device& device,
	const Vulkan::Command_Pool& temporary_command_pool, const glm::fvec2& scale,
	const glm::fvec2& translation, const glm::fvec3& color) : scale(scale),
	translation(translation), color(color)
{
	//Vertex data
	if(vertex_buffer.get()) return;
	vertex_buffer = {device, temporary_command_pool, vk::BufferUsageFlagBits::eVertexBuffer,
		sizeof(float)*VERTICES.size(), VERTICES.data()};
	index_buffer = {device, temporary_command_pool, vk::BufferUsageFlagBits::eIndexBuffer,
		sizeof(uint16_t)*INDICES.size(), INDICES.data()};
}

void Oreginum::Rectangle::draw(const Vulkan::Pipeline& pipeline,
	const Vulkan::Command_Buffer& command_buffer)
{
	uniforms.color = color;
	uniforms.matrix = glm::ortho(0.0f, (float)Oreginum::Window::get_resolution().x, 0.0f,
		(float)Oreginum::Window::get_resolution().y, -1.0f, 1.0f);
	uniforms.matrix *= glm::translate(glm::fvec3{translation, 0});
	uniforms.matrix *= glm::scale(glm::fvec3{scale, 0});

	command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});
	command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint16);
	command_buffer.get().pushConstants(pipeline.get_layout(),
		vk::ShaderStageFlagBits::eVertex, 0, sizeof(Uniforms), &uniforms);
	command_buffer.get().drawIndexed(static_cast<uint32_t>(VERTICES.size()), 1, 0, 0, 0);
}