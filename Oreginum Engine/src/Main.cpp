#define NOMINMAX
#include <windows.h>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtx/transform.hpp>
#include "Oreginum/Core.hpp"
#include "Oreginum/Window.hpp"
#include "Oreginum/Keyboard.hpp"
#include "Oreginum/Rectangle.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/Command Buffer.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/Image.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/Descriptor Set.hpp"
#include "Vulkan/Render Pass.hpp"
#include "Vulkan/Shader.hpp"
#include "Vulkan/Framebuffer.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Semaphore.hpp"
#include "Vulkan/Fence.hpp"

/*
	Use uint32_t vertices.
	Reduce the number of device passes.
	See about consolidating command pool and command buffer classes.
	Rename Command Pool to Command Buffer Pool.
	See about removing single time command pool.

	See about allocating image and buffer memory in larger chunks.
	Revise image layouts.
	Look into using multiple queues.
	Look into multithreading command buffer creation.
	Use push constants for uniforms.
	Optimize image creation.
*/

namespace
{
	Oreginum::Vulkan::Instance instance;
	Oreginum::Vulkan::Surface surface;
	Oreginum::Vulkan::Device device;
	Oreginum::Vulkan::Swapchain swapchain;
	Oreginum::Vulkan::Render_Pass render_pass;
	Oreginum::Vulkan::Shader shader;
	Oreginum::Vulkan::Pipeline pipeline;
	std::vector<Oreginum::Vulkan::Framebuffer> framebuffers;
	Oreginum::Vulkan::Command_Pool command_pool, temporary_command_pool;
	std::vector<Oreginum::Vulkan::Command_Buffer> command_buffers;
	Oreginum::Vulkan::Semaphore image_available, render_finished;

	Oreginum::Rectangle fruit;
	std::vector<Oreginum::Rectangle> snake;
	glm::fvec3 background_color{.1f, .1f, .1f},
		fruit_color{1.f, .1f, .3f}, snake_color{.3f, 1.f, .5f};
	enum class Direction{UP, DOWN, LEFT, RIGHT} direction, previous_direction;
	constexpr int board_size{465}, tile_size{15}, tiles{board_size/tile_size};
	float snake_timer, move_time;
}

void initialize_vulkan_static()
{
	instance = {};
	surface = {instance};
	device = {instance, surface};
	swapchain = {instance, surface, &device};
	temporary_command_pool = {device, device.get_graphics_queue_family_index(), 
		vk::CommandPoolCreateFlagBits::eTransient};
	command_pool = {device, device.get_graphics_queue_family_index()};
	image_available = {device};
	render_finished = {device};
}

void initialize_vulkan_volatile()
{
	device.get().waitIdle();
	render_pass = {device};

	shader = {device, {{"Snake Vertex", vk::ShaderStageFlagBits::eVertex},
	{"Snake Fragment", vk::ShaderStageFlagBits::eFragment}}};
	pipeline = {device, swapchain, render_pass, shader,
		Oreginum::Rectangle::get_uniforms_size()};

	framebuffers.clear();
	for(const auto& i : swapchain.get_images())
		framebuffers.push_back({device, swapchain, render_pass, i});

	command_buffers.clear();
	for(int i{}; i < framebuffers.size(); ++i)
	{
		command_buffers.push_back({device, command_pool});

		vk::CommandBufferBeginInfo command_buffer_begin_information;
		command_buffer_begin_information.setFlags(
			vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		command_buffers.back().get().begin(command_buffer_begin_information);

		std::array<vk::ClearValue, 1> clear_values{std::array<float, 4>{
			background_color.r, background_color.g, background_color.b, 1}};
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

		fruit.draw(pipeline, command_buffers.back());
		for(Oreginum::Rectangle& s : snake)
			s.draw(pipeline, command_buffers.back());

		command_buffers.back().get().endRenderPass();

		if(command_buffers.back().get().end() != vk::Result::eSuccess)
			Oreginum::Core::error("Could not record a Vulkan command buffer.");
	}
}

void reinitialize_swapchain()
{
	swapchain.reinitialize(&device);
	initialize_vulkan_volatile();
}

void set_fruit(){ fruit.set_translation({rand()%tiles*tile_size, rand()%tiles*tile_size}); }

void set()
{
	snake.clear();
	snake.push_back({device, temporary_command_pool, {tile_size,
		tile_size}, glm::ivec2{tiles/2*tile_size}, snake_color});
	set_fruit();
	initialize_vulkan_volatile();

	direction = Direction::RIGHT;
	previous_direction = direction;
	snake_timer = 0;
	move_time = .1f;
}

int WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	Oreginum::Core::initialize("Oreginum Engine Vulkan Test", glm::ivec2{board_size});
	initialize_vulkan_static();
	fruit = Oreginum::Rectangle{device, temporary_command_pool,
		{tile_size, tile_size}, {}, fruit_color};
	set();

	//Program
	while(Oreginum::Core::update())
	{
		//Controls
		if(Oreginum::Keyboard::was_pressed(Oreginum::Key::W)
			&& previous_direction != Direction::DOWN) direction = Direction::UP;
		else if(Oreginum::Keyboard::was_pressed(Oreginum::Key::S)
			&& previous_direction != Direction::UP) direction = Direction::DOWN;
		else if(Oreginum::Keyboard::was_pressed(Oreginum::Key::A)
			&& previous_direction != Direction::RIGHT) direction = Direction::LEFT;
		else if(Oreginum::Keyboard::was_pressed(Oreginum::Key::D)
			&& previous_direction != Direction::LEFT) direction = Direction::RIGHT;

		//Move snake
		if(snake_timer < move_time) snake_timer += Oreginum::Core::get_delta();
		else
		{
			//Eat fruit
			if(snake.back().get_translation() == fruit.get_translation())
			{
				move_time /= 1.01f;
				snake.push_back({device, temporary_command_pool, {tile_size,
					tile_size}, snake.back().get_translation(), snake_color});
				set_fruit();
			}

			//Move
			for(int i{}; i < snake.size()-1; ++i)
				snake[i].set_translation(snake[i+1].get_translation());

			if(direction == Direction::UP) snake.back().translate({0, -tile_size});
			else if(direction == Direction::DOWN) snake.back().translate({0, tile_size});
			else if(direction == Direction::LEFT) snake.back().translate({-tile_size, 0});
			else if(direction == Direction::RIGHT) snake.back().translate({tile_size, 0});

			initialize_vulkan_volatile();
			previous_direction = direction;
			snake_timer = 0;
		}

		//Game over
		glm::fvec2 head{snake.back().get_translation()};
		if(head.x < 0 || head.x >= board_size || head.y < 0 || head.y >= board_size) set();
		for(int i{}; i < snake.size()-1; ++i) if(head == snake[i].get_translation()) set();

		//Render
		if(Oreginum::Window::was_resized()) reinitialize_swapchain();

		uint32_t image_index;
		vk::Result result{device.get().acquireNextImageKHR(swapchain.get(),
			std::numeric_limits<uint64_t>::max(), image_available.get(),
			VK_NULL_HANDLE, &image_index)};
		if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) 
			reinitialize_swapchain(); else if(result != vk::Result::eSuccess)
			Oreginum::Core::error("Could not aquire a Vulkan swapchain image.");

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
			vk::Result::eSuccess) Oreginum::Core::error("Could not submit Vulkan"
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
			Oreginum::Core::error("Could not submit Vulkan presentation queue.");
	}

	device.get().waitIdle();
}