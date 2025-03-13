#pragma once
#include "Vulkan/ApplicationRZ.h"
#include "Vulkan/XVKFrameBuffer.h"
namespace xvk::ray
{
	class ApplicationRT : public Application
	{

	protected:
		void CreateAccelerationStructures() {}
		void DeleteAccelerationStructures() {}

	public:
		ApplicationRT(const WindowState& windowState, VkPresentModeKHR presentMode,
			bool enableValidationLayer);
		int test;
	};
}

