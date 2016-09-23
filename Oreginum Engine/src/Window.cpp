#include "Error.hpp"
#include "Window.hpp"

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
	Window *origin{reinterpret_cast<Window *>(glfwGetWindowUserPointer(window))};
	origin->resized = true;
	origin->resolution = {width, height};
}

Window::Window(const std::string& title, const glm::ivec2& resolution)
	: TITLE(title), resolution(resolution)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(resolution.x, resolution.y, TITLE.c_str(), nullptr, nullptr);
	if(!window) error("Could not create GLFW window.");
	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, resize_callback);
}

Window::~Window(){ glfwTerminate(); }

void Window::update()
{
	resized = false;
	glfwPollEvents();

}