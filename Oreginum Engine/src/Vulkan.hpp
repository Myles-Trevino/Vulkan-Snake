#pragma once
#include <vector>
#include <functional>
#define VK_USE_PLATFORM_WIN32_KHR
#include <Vulkan/vulkan.h>
#include "Window.hpp"

template <typename T> class Vulkan_Deleter
{
public:
	Vulkan_Deleter() : Vulkan_Deleter([](T, VkAllocationCallbacks *){}){}
	Vulkan_Deleter(std::function<void(T, VkAllocationCallbacks *)> deletef)
	{ this->deleter = [=](T obj){ deletef(obj, nullptr); }; }
	Vulkan_Deleter(const Vulkan_Deleter<VkInstance>& instance,
		std::function<void(VkInstance, T, VkAllocationCallbacks *)> deletef)
	{ this->deleter = [&instance, deletef](T obj){ deletef(instance, obj, nullptr); }; }
	Vulkan_Deleter(const Vulkan_Deleter<VkDevice>& device,
		std::function<void(VkDevice, T, VkAllocationCallbacks *)> deletef)
	{ this->deleter = [&device, deletef](T obj){ deletef(device, obj, nullptr); }; }
	~Vulkan_Deleter(){ cleanup(); }
	T *operator &(){ cleanup(); return &object; }
	T *replace(){ cleanup(); return &object; }
	operator T() const { return object; }

private:
	T object{VK_NULL_HANDLE};
	std::function<void(T)> deleter;
	void cleanup(){ if(object != VK_NULL_HANDLE) deleter(object); object = VK_NULL_HANDLE; }
};

class Vulkan
{
public:
	Vulkan(const Window& window, const std::string& program_title,
		const glm::ivec3& program_version, const std::string& engine_title,
		const glm::ivec3& engine_version, const glm::ivec3& vulkan_version,
		const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions,
		const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions,
		const std::vector<float>& vertices, bool debug = false);
	~Vulkan(){ vkDeviceWaitIdle(device); }

	void render();
	void recreate_swapchain();

private:
	const Window& WINDOW;
	const bool DEBUG;
	const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions;
	const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions;
	const std::vector<float>& vertices;
	Vulkan_Deleter<VkInstance> instance{vkDestroyInstance};
	Vulkan_Deleter<VkDebugReportCallbackEXT> debug_callback{instance, destroy_debug_callback};
	Vulkan_Deleter<VkSurfaceKHR> surface{instance, vkDestroySurfaceKHR};
	std::vector<const char *> gpu_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkPhysicalDevice gpu;
	uint32_t graphics_queue_index;
	uint32_t present_queue_index;
	VkPhysicalDeviceProperties gpu_properties;
	VkPhysicalDeviceFeatures gpu_features;
	VkSurfaceCapabilitiesKHR swapchain_capabilities;
	std::vector<VkSurfaceFormatKHR> swapchain_formats;
	std::vector<VkPresentModeKHR> swapchain_present_modes;
	Vulkan_Deleter<VkDevice> device{vkDestroyDevice};
	VkQueue graphics_queue;
	VkQueue present_queue;
	Vulkan_Deleter<VkSwapchainKHR> swapchain{device, vkDestroySwapchainKHR};
	VkExtent2D swapchain_extent;
	std::vector<VkImage> swapchain_images;
	std::vector<Vulkan_Deleter<VkImageView>> swapchain_image_views;
	Vulkan_Deleter<VkRenderPass> render_pass{device, vkDestroyRenderPass};
	Vulkan_Deleter<VkPipelineLayout> pipeline_layout{device, vkDestroyPipelineLayout};
	Vulkan_Deleter<VkPipeline> pipeline{device, vkDestroyPipeline};
	std::vector<Vulkan_Deleter<VkFramebuffer>> framebuffers;
	Vulkan_Deleter<VkCommandPool> command_pool{device, vkDestroyCommandPool};
	Vulkan_Deleter<VkBuffer> vertex_buffer{device, vkDestroyBuffer};
	Vulkan_Deleter<VkDeviceMemory> vertex_buffer_memory{device, vkFreeMemory};
	std::vector<VkCommandBuffer> command_buffers;
	Vulkan_Deleter<VkSemaphore> image_available_semaphore{device, vkDestroySemaphore};
	Vulkan_Deleter<VkSemaphore> render_finished_semaphore{device, vkDestroySemaphore};

	void create_instance(const Window& window, const std::string& program_title,
		const glm::ivec3& program_version, const std::string& engine_title,
		const glm::ivec3& engine_version, const glm::ivec3& vulkan_version);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback_function(VkDebugReportFlagsEXT flags,	
		VkDebugReportObjectTypeEXT type, uint64_t object, size_t location, int32_t code,
		const char *layer_prefix, const char *message, void *user_data);
	void create_debug_callback();
	static void destroy_debug_callback(VkInstance instance, VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks *allocator);
	void create_surface();
	void get_swapchain_information(const VkPhysicalDevice& gpu);
	void get_gpu_information(const VkPhysicalDevice& gpu);
	void select_gpu();
	void create_device();
	void create_swapchain();
	void create_image_views();
	void create_render_pass();
	Vulkan_Deleter<VkShaderModule> create_shader(const std::string& shader);
	void create_graphics_pipeline();
	void create_framebuffers();
	void create_command_pool();
	uint32_t find_memory(uint32_t type, VkMemoryPropertyFlags properties);
	void create_vertex_buffer();
	void create_command_buffers();
	void create_semaphores();
};