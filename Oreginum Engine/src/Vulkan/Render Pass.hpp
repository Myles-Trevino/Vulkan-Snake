#pragma once
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Render_Pass
	{
	public:
		Render_Pass(){};
		~Render_Pass(){ destroy(); }

		void initialize(const Device *device);

		VkRenderPass get() const { return render_pass; }

	private:
		const Device *device;

		VkRenderPass render_pass;

		void destroy();
	};
}