#pragma once
#include <Vulkan/vulkan.hpp>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Sampler
	{
	public:
		Sampler(){};
		Sampler(const Device& device, vk::Filter filter = vk::Filter::eLinear,
			vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode address_mode = vk::SamplerAddressMode::eRepeat);
		Sampler *operator=(Sampler other){ swap(&other); return this; }
		~Sampler(){ if(sampler.unique() && *sampler) device->get().destroySampler(*sampler); };

		const vk::Sampler& get() const { return *sampler; }

	private:
		const Device *device;
		std::shared_ptr<vk::Sampler> sampler = std::make_shared<vk::Sampler>();

		void swap(Sampler *other);
	};
}