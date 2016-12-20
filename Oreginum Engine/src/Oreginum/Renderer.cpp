#include "../Vulkan/Pipeline.hpp"
#include "../Vulkan/Framebuffer.hpp"
#include "../Vulkan/Semaphore.hpp"
#include "Core.hpp"
#include "Window.hpp"
#include "Renderer.hpp"

namespace
{
	Oreginum::Vulkan::Instance instance;
	Oreginum::Vulkan::Surface surface;
	Oreginum::Vulkan::Device device;
	Oreginum::Vulkan::Swapchain swapchain;
	Oreginum::Vulkan::Render_Pass render_pass;
	Oreginum::Vulkan::Shader shader;
	Oreginum::Vulkan::Descriptor_Pool descriptor_pool;
	Oreginum::Vulkan::Descriptor_Set descriptor_set;
	Oreginum::Vulkan::Buffer uniform_buffer;
	Oreginum::Vulkan::Pipeline pipeline;
	std::vector<Oreginum::Vulkan::Framebuffer> framebuffers;
	Oreginum::Vulkan::Command_Pool command_pool, temporary_command_pool;
	std::vector<Oreginum::Vulkan::Command_Buffer> command_buffers;
	Oreginum::Vulkan::Command_Buffer temporary_command_buffer;
	Oreginum::Vulkan::Semaphore image_available, render_finished;

	const glm::fvec3 BACKGROUND_COLOR{.1f, .1f, .1f};
	std::vector<Oreginum::Renderable *> renderables;
	uint32_t padded_uniform_size, uniform_buffer_size;

	bool rerecord{true};
}

void Oreginum::Renderer::initialize(bool debug)
{
	instance = {debug};
	surface = {instance};
	device = {instance, surface};
	swapchain = {instance, surface, &device};
	image_available = {device};
	render_finished = {device};
	temporary_command_pool = {device, device.get_graphics_queue_family_index(),
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer};
	temporary_command_buffer = {device, temporary_command_pool};
	command_pool = {device, device.get_graphics_queue_family_index()};
	descriptor_pool = {device, {{vk::DescriptorType::eUniformBufferDynamic, 1}}};

	//Calculate uniform buffer padding
	uint32_t minimum_offset{
		static_cast<uint32_t>(device.get_properties().limits.minUniformBufferOffsetAlignment)};
	uint32_t uniform_size{static_cast<uint32_t>(sizeof(Renderable::Uniforms))};
	uint32_t offset_difference{uniform_size%minimum_offset};
	if(offset_difference) padded_uniform_size = uniform_size+(minimum_offset-offset_difference);
}

void Oreginum::Renderer::add(Renderable *renderable)
{
	renderables.push_back(renderable);
	rerecord = true;
}

void Oreginum::Renderer::add(const std::vector<Renderable *>& renderables)
{
	for(Renderable * r : renderables) ::renderables.push_back(r);
	rerecord = true;
}

void Oreginum::Renderer::remove(uint32_t index)
{
	renderables.erase(renderables.begin()+index);
	rerecord = true;
}

void Oreginum::Renderer::clear()
{
	renderables.clear();
	rerecord = true;
}

void Oreginum::Renderer::record()
{
	rerecord = false;
	device.get().waitIdle();

	if(!renderables.empty())
	{
		uniform_buffer_size = static_cast<uint32_t>(renderables.size()-1)*
			padded_uniform_size+sizeof(Renderable::Uniforms);
		device.get().resetCommandPool(temporary_command_pool.get(), vk::CommandPoolResetFlags{});
		uniform_buffer = {device, temporary_command_buffer,
			vk::BufferUsageFlagBits::eUniformBuffer, uniform_buffer_size};
	}

	device.get().resetDescriptorPool(descriptor_pool.get());
	descriptor_set = {device, descriptor_pool, vk::DescriptorType::eUniformBufferDynamic,
		vk::ShaderStageFlagBits::eVertex, uniform_buffer, sizeof(Renderable::Uniforms)};
	render_pass = {device};

	shader = {device, {{"3D Vertex", vk::ShaderStageFlagBits::eVertex},
		{"3D Fragment", vk::ShaderStageFlagBits::eFragment}}};
	pipeline = {device, swapchain, render_pass, shader, descriptor_set};

	framebuffers.clear();
	for(const auto& i : swapchain.get_images())
		framebuffers.push_back({device, swapchain, render_pass, i});

	command_buffers.clear();
	device.get().resetCommandPool(command_pool.get(), vk::CommandPoolResetFlags{});
	for(int i{}; i < framebuffers.size(); ++i)
	{
		command_buffers.push_back({device, command_pool});
		command_buffers.back().begin();

		std::array<vk::ClearValue, 1> clear_values{std::array<float, 4>{
			BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1}};
		vk::RenderPassBeginInfo render_pass_begin_information;
		render_pass_begin_information.setRenderPass(render_pass.get());
		render_pass_begin_information.setFramebuffer(framebuffers[i].get());
		render_pass_begin_information.setRenderArea(vk::Rect2D{{0, 0}, swapchain.get_extent()});
		render_pass_begin_information.setClearValueCount(
			static_cast<uint32_t>(clear_values.size()));
		render_pass_begin_information.setPClearValues(clear_values.data());

		command_buffers.back().get().beginRenderPass(
			render_pass_begin_information, vk::SubpassContents::eInline);
		command_buffers.back().get().bindPipeline(
			vk::PipelineBindPoint::eGraphics, pipeline.get());
		for(int j{}; j < renderables.size(); ++j) renderables[j]->draw(descriptor_set,
			pipeline, command_buffers.back(), j*padded_uniform_size);
		command_buffers.back().get().endRenderPass();

		command_buffers.back().end();
	}
}

void Oreginum::Renderer::reinitialize_swapchain()
{
	device.get().waitIdle();
	swapchain.reinitialize(&device);
	record();
}

void Oreginum::Renderer::render()
{
	if(Window::was_resized()) reinitialize_swapchain();
	if(rerecord) record();

	if(!renderables.empty())
	{
		void *buffer = std::malloc(uniform_buffer_size);
		for(int i{}; i < renderables.size(); ++i)
		{
			renderables[i]->update();
			std::memcpy((void *)((char *)buffer+i*padded_uniform_size),
				&renderables[i]->get_uniforms(), sizeof(Renderable::Uniforms));
		}
		uniform_buffer.write(buffer, uniform_buffer_size);
		std::free(buffer);
	}

	uint32_t image_index;
	vk::Result result{device.get().acquireNextImageKHR(swapchain.get(),
		std::numeric_limits<uint64_t>::max(), image_available.get(),
		VK_NULL_HANDLE, &image_index)};
	if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
		reinitialize_swapchain(); else if(result != vk::Result::eSuccess)
		Core::error("Could not aquire a Vulkan swapchain image.");

	std::array<vk::Semaphore, 1> submit_wait_semaphores{image_available.get()};
	std::array<vk::PipelineStageFlags, 1> wait_stages
	{vk::PipelineStageFlagBits::eColorAttachmentOutput};
	std::array<vk::CommandBuffer, 1> submit_command_buffers
	{command_buffers[image_index].get()};
	std::array<vk::Semaphore, 1> submit_signal_semaphores{render_finished.get()};
	vk::SubmitInfo submit_information;
	submit_information.setWaitSemaphoreCount(
		static_cast<uint32_t>(submit_wait_semaphores.size()));
	submit_information.setPWaitSemaphores(submit_wait_semaphores.data());
	submit_information.setPWaitDstStageMask(wait_stages.data());
	submit_information.setCommandBufferCount(
		static_cast<uint32_t>(submit_command_buffers.size()));
	submit_information.setPCommandBuffers(submit_command_buffers.data());
	submit_information.setSignalSemaphoreCount(
		static_cast<uint32_t>(submit_signal_semaphores.size()));
	submit_information.setPSignalSemaphores(submit_signal_semaphores.data());

	if(device.get_graphics_queue().submit(submit_information, VK_NULL_HANDLE) !=
		vk::Result::eSuccess) Core::error("Could not submit Vulkan"
			"render command buffer.");

	std::array<vk::Semaphore, 1> present_wait_semaphores{render_finished.get()};
	std::array<vk::SwapchainKHR, 1> swapchains{swapchain.get()};
	vk::PresentInfoKHR present_information;
	present_information.setWaitSemaphoreCount(
		static_cast<uint32_t>(present_wait_semaphores.size()));
	present_information.setPWaitSemaphores(present_wait_semaphores.data());
	present_information.setSwapchainCount(static_cast<uint32_t>(swapchains.size()));
	present_information.setPSwapchains(swapchains.data());
	present_information.setPImageIndices(&image_index);
	present_information.setPResults(nullptr);

	result = device.get_graphics_queue().presentKHR(present_information);
	if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
		reinitialize_swapchain(); else if(result != vk::Result::eSuccess)
		Core::error("Could not submit Vulkan presentation queue.");
}

const Oreginum::Vulkan::Device& Oreginum::Renderer::get_device(){ return device; };

const Oreginum::Vulkan::Command_Buffer& Oreginum::Renderer::get_temporary_command_buffer()
{ return temporary_command_buffer; }