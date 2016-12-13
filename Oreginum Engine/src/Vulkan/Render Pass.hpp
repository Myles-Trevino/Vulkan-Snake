#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Render_Pass
	{
	public:
		Render_Pass(){}
		Render_Pass(const Device& device);
		Render_Pass *Render_Pass::operator=(Render_Pass render_pass)
		{ swap(&render_pass); return this; }
		~Render_Pass();

		const vk::RenderPass& get() const { return *render_pass; }

	private:
		const Device *device;
		std::shared_ptr<vk::RenderPass> render_pass = std::make_shared<vk::RenderPass>();

		void swap(Render_Pass *other);
	};
}