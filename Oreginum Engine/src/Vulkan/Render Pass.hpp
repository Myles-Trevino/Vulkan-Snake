#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Render_Pass
	{
	public:
		Render_Pass(const Device& device);
		~Render_Pass(){ device.get().destroyRenderPass(render_pass); }

		const vk::RenderPass& get() const { return render_pass; }

	private:
		const Device& device;

		vk::RenderPass render_pass;
	};
}