#include "../Vulkan/Pipeline.hpp"
#include "../Vulkan/Framebuffer.hpp"
#include "../Vulkan/Semaphore.hpp"
#include "../Vulkan/Sampler.hpp"
#include "Core.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"

namespace
{
	Oreginum::Vulkan::Instance instance;
	Oreginum::Vulkan::Surface surface;
	Oreginum::Vulkan::Device device;
	Oreginum::Vulkan::Swapchain swapchain;
	Oreginum::Vulkan::Render_Pass render_pass;
	Oreginum::Vulkan::Shader primitive_2d_shader, sprite_shader,
		primitive_3d_shader, model_shader, environment_shader;
	Oreginum::Vulkan::Descriptor_Pool descriptor_pool;
	Oreginum::Vulkan::Descriptor_Set untextured_descriptor_set, textured_descriptor_set;
	Oreginum::Vulkan::Buffer uniform_buffer;
	Oreginum::Vulkan::Pipeline primitive_2d_pipeline, sprite_pipeline,
		primitive_3d_pipeline, model_pipeline, environment_pipeline;
	std::vector<Oreginum::Vulkan::Framebuffer> framebuffers;
	Oreginum::Vulkan::Image depth_image;
	Oreginum::Vulkan::Command_Pool command_pool, temporary_command_pool;
	std::vector<Oreginum::Vulkan::Command_Buffer> command_buffers;
	Oreginum::Vulkan::Command_Buffer temporary_command_buffer;
	Oreginum::Vulkan::Semaphore image_available, render_finished;

	const glm::fvec3 BACKGROUND_COLOR{.1f, .1f, .1f};
	std::vector<Oreginum::Renderable *> renderables;
	uint32_t uniform_size, padded_uniform_size, uniform_buffer_size;

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

	//Calculate uniform buffer padding
	uint32_t minimum_offset{
		static_cast<uint32_t>(device.get_properties().limits.minUniformBufferOffsetAlignment)};
	uniform_size = static_cast<uint32_t>(sizeof(Renderable::Model_Uniforms));
	uint32_t offset_difference{uniform_size%minimum_offset};
	padded_uniform_size = uniform_size;
	if(offset_difference) padded_uniform_size += minimum_offset-offset_difference;
}

void Oreginum::Renderer::add(Renderable *renderable)
{ renderables.push_back(renderable), rerecord = true; }

void Oreginum::Renderer::add(const std::vector<Renderable *>& renderables)
{
	for(Renderable *r : renderables) ::renderables.push_back(r); rerecord = true;
}

void Oreginum::Renderer::remove(uint32_t index)
{ renderables.erase(renderables.begin()+index), rerecord = true; }

void Oreginum::Renderer::clear(){ renderables.clear(), rerecord = true; }

void Oreginum::Renderer::record()
{
	rerecord = false;
	device.get().waitIdle();

	//Uniform buffer
	if(!renderables.empty())
	{
		uniform_buffer_size = static_cast<uint32_t>(renderables.size())*padded_uniform_size;
		device.get().resetCommandPool(temporary_command_pool.get(), vk::CommandPoolResetFlags{});
		uniform_buffer = {device, temporary_command_buffer,
			vk::BufferUsageFlagBits::eUniformBuffer, uniform_buffer_size};
	}

	//Depth image
	depth_image = Vulkan::Image{device, swapchain.get_extent(),
		vk::ImageUsageFlagBits::eDepthStencilAttachment, Vulkan::Image::DEPTH_FORMAT,
		vk::ImageAspectFlagBits::eDepth};
	depth_image.transition(temporary_command_buffer, vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AccessFlags{},
		vk::AccessFlagBits::eDepthStencilAttachmentRead |
		vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	//Descriptors
	uint32_t textured_renderables{};
	for(Renderable *r : renderables)
		if(r->get_type() == Renderable::SPRITE || r->get_type() == Renderable::MODEL
			|| r->get_type() == Renderable::ENVIRONMENT)
			textured_renderables += r->get_meshes();

	descriptor_pool = {device, {{vk::DescriptorType::eUniformBufferDynamic, textured_renderables+2},
		{vk::DescriptorType::eCombinedImageSampler, textured_renderables+1}}};

	untextured_descriptor_set = {device, descriptor_pool,
		{{vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex}}};
	textured_descriptor_set = {device, descriptor_pool,
		{{vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex},
		{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}};

	for(Renderable *r : renderables) r->initialize_descriptor();
	if(!renderables.empty())
	{
		vk::DescriptorBufferInfo buffer_information{uniform_buffer.get(), 0, padded_uniform_size};
		untextured_descriptor_set.write({{vk::DescriptorType::eUniformBufferDynamic,
			&buffer_information, nullptr}});
	}

	//Render pass
	render_pass = {device};

	//Pipelines
	primitive_2d_shader = {device, {{"Primitive 2D Vertex", vk::ShaderStageFlagBits::eVertex},
		{"Primitive 2D Fragment", vk::ShaderStageFlagBits::eFragment}}};
	sprite_shader = {device, {{"Sprite Vertex", vk::ShaderStageFlagBits::eVertex},
		{"Sprite Fragment", vk::ShaderStageFlagBits::eFragment}}};
	primitive_3d_shader = {device, {{"Primitive 3D Vertex", vk::ShaderStageFlagBits::eVertex},
		{"Primitive 3D Fragment", vk::ShaderStageFlagBits::eFragment}}};
	model_shader = {device, {{"Model Vertex", vk::ShaderStageFlagBits::eVertex},
		{"Model Fragment", vk::ShaderStageFlagBits::eFragment}}};
	environment_shader = {device, {{"Environment Vertex", vk::ShaderStageFlagBits::eVertex},
		{"Environment Fragment", vk::ShaderStageFlagBits::eFragment}}};

	primitive_2d_pipeline = {device, swapchain, render_pass,
		primitive_2d_shader, untextured_descriptor_set};
	sprite_pipeline = {device, swapchain, render_pass, sprite_shader,
		textured_descriptor_set, false, false, primitive_2d_pipeline};
	primitive_3d_pipeline = {device, swapchain, render_pass, primitive_3d_shader,
		untextured_descriptor_set, true, false, primitive_2d_pipeline};
	model_pipeline = {device, swapchain, render_pass, model_shader,
		textured_descriptor_set, true, true, primitive_3d_pipeline};
	environment_pipeline = {device, swapchain, render_pass, environment_shader,
		textured_descriptor_set, false, false, sprite_pipeline};

	//Framebuffers
	framebuffers.clear();
	for(const auto& i : swapchain.get_images())
		framebuffers.push_back({device, swapchain, render_pass, i, depth_image});

	//Command buffers
	command_buffers.clear();
	device.get().resetCommandPool(command_pool.get(), vk::CommandPoolResetFlags{});
	for(int i{}; i < framebuffers.size(); ++i)
	{
		command_buffers.push_back({device, command_pool});
		command_buffers.back().begin();

		std::array<vk::ClearValue, 2> clear_values{};
		clear_values[0].setColor(std::array<float, 4>{
			BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, 1});
		clear_values[1].setDepthStencil({1, 0});

		vk::RenderPassBeginInfo render_pass_begin_information;
		render_pass_begin_information.setRenderPass(render_pass.get());
		render_pass_begin_information.setFramebuffer(framebuffers[i].get());
		render_pass_begin_information.setRenderArea(vk::Rect2D{{0, 0}, swapchain.get_extent()});
		render_pass_begin_information.setClearValueCount(
			static_cast<uint32_t>(clear_values.size()));
		render_pass_begin_information.setPClearValues(clear_values.data());

		command_buffers.back().get().beginRenderPass(
			render_pass_begin_information, vk::SubpassContents::eInline);

		for(int j{}; j < renderables.size(); ++j)
			if(renderables[j]->get_type() == Renderable::SPRITE ||
				renderables[j]->get_type() == Renderable::MODEL ||
				renderables[j]->get_type() == Renderable::ENVIRONMENT)
				renderables[j]->draw(command_buffers.back(), j*padded_uniform_size);
			else 
				renderables[j]->draw(untextured_descriptor_set,
					command_buffers.back(), j*padded_uniform_size);

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
			std::memcpy((char *)buffer+i*padded_uniform_size,
				renderables[i]->get_uniforms(), renderables[i]->get_uniforms_size());
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

	result = device.get_present_queue().presentKHR(present_information);
	if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
		reinitialize_swapchain(); else if(result != vk::Result::eSuccess)
		Core::error("Could not submit Vulkan presentation queue.");
}

const Oreginum::Vulkan::Device& Oreginum::Renderer::get_device(){ return device; };

const Oreginum::Vulkan::Command_Buffer& Oreginum::Renderer::get_temporary_command_buffer()
{ return temporary_command_buffer; }

const Oreginum::Vulkan::Descriptor_Pool& Oreginum::Renderer::get_descriptor_pool()
{ return descriptor_pool; }

const Oreginum::Vulkan::Buffer& Oreginum::Renderer::get_uniform_buffer()
{ return uniform_buffer; }

uint32_t Oreginum::Renderer::get_padded_uniform_size(){ return padded_uniform_size; }

const Oreginum::Vulkan::Pipeline& Oreginum::Renderer::get_primitive_2d_pipeline()
{ return primitive_2d_pipeline; }

const Oreginum::Vulkan::Pipeline& Oreginum::Renderer::get_sprite_pipeline()
{ return sprite_pipeline; }

const Oreginum::Vulkan::Pipeline& Oreginum::Renderer::get_primitive_3d_pipeline()
{ return primitive_3d_pipeline; }

const Oreginum::Vulkan::Pipeline& Oreginum::Renderer::get_model_pipeline()
{ return model_pipeline; }

const Oreginum::Vulkan::Pipeline& Oreginum::Renderer::get_environment_pipeline()
{ return environment_pipeline; }