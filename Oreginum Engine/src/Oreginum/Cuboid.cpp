#include "Camera.hpp"
#include "Renderer.hpp"
#include "Cuboid.hpp"

Oreginum::Vulkan::Buffer Oreginum::Cuboid::vertex_buffer;
Oreginum::Vulkan::Buffer Oreginum::Cuboid::index_buffer;

Oreginum::Cuboid::Cuboid(const glm::fvec3& translation, const glm::fvec3& scale,
	const glm::fvec3& color, bool center) : scale(scale), translation(translation),
	uniforms({{}, color}), center(center)
{
	if(vertex_buffer.get()) return;
	vertex_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eVertexBuffer, sizeof(float)*VERTICES.size(), VERTICES.data()};
	index_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eIndexBuffer, sizeof(uint16_t)*INDICES.size(), INDICES.data()};
}

void Oreginum::Cuboid::draw(const Vulkan::Descriptor_Set& descriptor_set,
	const Vulkan::Pipeline& pipeline, const Vulkan::Command_Buffer& command_buffer,
	uint32_t offset)
{
	command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});
	command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint16);
	command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		pipeline.get_layout(), 0, {descriptor_set.get()}, {offset});
	command_buffer.get().drawIndexed(static_cast<uint32_t>(INDICES.size()), 1, 0, 0, 0);
}

void Oreginum::Cuboid::update()
{
	uniforms.matrix = Camera::get_projection()*Camera::get_view()*
		glm::translate(translation)*rotation*glm::scale(scale);
	if(center) uniforms.matrix *= glm::translate(glm::fvec3{-.5f, -.5f, -.5f});
}