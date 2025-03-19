#include "ApplicationRT.h"

namespace xvk::ray
{
	ApplicationRT::ApplicationRT(const WindowState& windowState, VkPresentModeKHR presentMode,
		bool enableValidationLayer)
		:Application(windowState, presentMode, enableValidationLayer)
	{
	}

	ApplicationRT::~ApplicationRT()
	{
		Application::DeleteSwapChain();
		DeleteAccelerationStructures();

		m_rayTracingContext.reset();
		m_deviceFunc.reset();
	}

	void ApplicationRT::CreateAccelerationStructures()
	{

	}

	void ApplicationRT::DeleteAccelerationStructures()
	{

	}

	void ApplicationRT::CreateSwapChain()
	{

	}

	void ApplicationRT::DeleteSwapChain()
	{

	}

	void ApplicationRT::Render(VkCommandBuffer commandBuffer, size_t currentFrame, uint32_t imageIndex)
	{

	}

	void ApplicationRT::CreateBLAS(VkCommandBuffer commandBuffer)
	{

	}

	void ApplicationRT::CreateTLAS(VkCommandBuffer commandBuffer)
	{

	}
	void ApplicationRT::CreateOutputImage()
	{

	}
}