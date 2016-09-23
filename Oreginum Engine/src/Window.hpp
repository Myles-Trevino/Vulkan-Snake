#pragma once
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>

class Window
{
public:
	Window(const std::string& title, const glm::ivec2& resolution);
	~Window();

	void update();

	GLFWwindow *get() const { return window; }
	std::string get_title() const { return TITLE; }
	glm::ivec2 get_resolution() const { return resolution; }
	bool has_resolution() const { return !(get_resolution().x && get_resolution().y); }
	bool was_closed() const { return glfwWindowShouldClose(window) != 0; }
	bool was_resized() const { return resized; }

private:
	GLFWwindow *window;
	const std::string TITLE;
	glm::ivec2 resolution;
	bool resized;

	static void resize_callback(GLFWwindow* window, int width, int height);
};