#include "VkWindow.h"

namespace xvk
{
	void GlfwErrorDebug(const int error, const char* const description)
	{
		std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
	}

	void GLFWKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
	{
		auto* const this_ = static_cast<XVKWindow*>(glfwGetWindowUserPointer(window));
		if (this_->OnKeyPressed)
		{
			this_->OnKeyPressed(key, scancode, action, mods);
		}
	}

	void GLFWCursorPosCallback(GLFWwindow* window, const double xpos, const double ypos)
	{
		auto* const this_ = static_cast<XVKWindow*>(glfwGetWindowUserPointer(window));
		if (this_->OnCursorPos)
		{
			this_->OnCursorPos(xpos, ypos);
		}
	}

	void GLFWMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
	{
		auto* const this_ = static_cast<XVKWindow*>(glfwGetWindowUserPointer(window));
		if (this_->OnMouseButtonClicked)
		{
			this_->OnMouseButtonClicked(button, action, mods);
		}
	}

	void GLFWScrollCallback(GLFWwindow* window, const double x, const double y)
	{
		auto* const this_ = static_cast<XVKWindow*>(glfwGetWindowUserPointer(window));
		if (this_->OnScroll)
		{
			this_->OnScroll(x, y);
		}
	}

	XVKWindow::XVKWindow(const WindowState& state)
		:windowState(state)
	{
		glfwSetErrorCallback(GlfwErrorDebug);
		if (!glfwInit())
		{
			RUNTIME_ERROR("Fail to init GLFW");
		}
		if (!glfwVulkanSupported())
		{
			RUNTIME_ERROR("GLFW dosen't support Vulkan");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, state.resizable ? GLFW_TRUE : GLFW_FALSE);

		vk_window = glfwCreateWindow(state.width, state.height, state.title.c_str()
			, nullptr, nullptr);
		if (vk_window == nullptr)
		{
			RUNTIME_ERROR("Fail to create GLFW window");
		}

		// set up callbacks
		glfwSetWindowUserPointer(vk_window, this);
		glfwSetCursorPosCallback(vk_window, GLFWCursorPosCallback);
		glfwSetMouseButtonCallback(vk_window, GLFWMouseButtonCallback);
		glfwSetKeyCallback(vk_window, GLFWKeyCallback);
		glfwSetScrollCallback(vk_window, GLFWScrollCallback);
	}

	XVKWindow::~XVKWindow()
	{
		if (vk_window != nullptr)
		{
			glfwDestroyWindow(vk_window);
			vk_window = nullptr;
		}

		glfwTerminate();
		glfwSetErrorCallback(nullptr);
	}

	VkExtent2D XVKWindow::FramebufferSize() const
	{
		int width, height;
		glfwGetWindowSize(vk_window, &width, &height);
		return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	VkExtent2D XVKWindow::WindowSize() const
	{
		int width, height;
		glfwGetWindowSize(vk_window, &width, &height);
		return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	const char* XVKWindow::GetKeyName(const int key, const int scancode) const
	{
		return glfwGetKeyName(key, scancode);
	}

	std::vector<const char*> XVKWindow::GetRequiredInstanceExtensions() const
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		return extensions;
	}

	void XVKWindow::Run()
	{
		glfwSetTime(0.0);

		while (!glfwWindowShouldClose(vk_window))
		{
			glfwPollEvents();

			if (DrawFrame)
			{
				DrawFrame();
			}
		}
	}

	void XVKWindow::WaitForEvents()
	{
		glfwWaitEvents();
	}
	
	void XVKWindow::Close()
	{
		glfwSetWindowShouldClose(vk_window, GLFW_TRUE);
	}

	double XVKWindow::GetTime()
	{
		return glfwGetTime();
	}

}