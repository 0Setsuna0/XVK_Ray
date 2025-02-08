#pragma once

#include <functional>
#include "VkInstance.h"
#include "VkCommon.h"

namespace xvk
{
	struct WindowState
	{
		std::string title;
		uint32_t width;		
		uint32_t height;
		bool cursorDisabled;
		bool resizable;
	};

	class XVKWindow
	{
	public:
		explicit XVKWindow(const WindowState& state);
		~XVKWindow();

		const WindowState& GetWindowState() const { return windowState; }
		GLFWwindow* Handle() const { return vk_window; }

		VkExtent2D FramebufferSize() const;
		VkExtent2D WindowSize() const;

		std::vector<const char*> GetRequiredInstanceExtensions() const;

		//callback
		std::function<void()> DrawFrame;
		std::function<void(int key, int scancode, int action, int mods)> OnKeyPressed;
		std::function<void(double xpos, double ypos)> OnCursorPos;
		std::function<void(int button, int action, int mods)> OnMouseButtonClicked;
		std::function<void(double xoffset, double yoffset)> OnScroll;

		const char* GetKeyName(const int key, const int scancode) const;

		void Run();
		void WaitForEvents();
		void Close();
		double GetTime();

	private:
		WindowState windowState;
		GLFWwindow* vk_window{};
	};
}