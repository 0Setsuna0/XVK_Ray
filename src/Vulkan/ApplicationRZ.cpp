#include "ApplicationRZ.h"
#include "XVKWindow.h"
#include "XVKBuffer.h"
#include "XVKCommandPool.h"
#include "XVKCommandBuffers.h"
#include "XVKDevice.h"
#include "XVKSwapChain.h"
#include "XVKRenderPass.h"
#include "XVKPipelineLayout.h"
#include "XVKGraphicsPipeline.h"
#include "XVKSemaphore.h"
#include "XVKFence.h"
#include "XVKFrameBuffer.h"
#include "XVKInstance.h"
#include "XVKDepthBuffer.h"
#include "XVKImage.h"
#include "XVKImageView.h"

#include "Asset/UniformBuffer.h"
#include "Asset/Scene.h"
namespace xvk
{
	ApplicationRZ::ApplicationRZ(const WindowState& windowState, VkPresentModeKHR presentMode,
		bool enableValidationLayer)
		:m_presentMode(presentMode)
	{
		const auto validationLayers = enableValidationLayer ?
			std::vector<const char*>{"VK_LAYER_KHRONOS_validation"} : std::vector<const char*>{};
		
		m_window.reset(new XVKWindow(windowState));
		m_instance.reset(new XVKInstance(*m_window, validationLayers));
		std::vector<const char*> requiredExtension{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		SetDevice(requiredExtension);
		SetCommandPool();
		CreateSwapChain();
	}

	ApplicationRZ::~ApplicationRZ()
	{
		DeleteSwapChain();
		m_commandPool.reset();
		m_device.reset();
		m_instance.reset();
		m_window.reset();
	}

	//----init----
	void ApplicationRZ::SetDevice(const std::vector<const char*>& requiredExtensions)
	{
		m_device.reset(new XVKDevice(*m_instance, requiredExtensions));
	}

	void ApplicationRZ::SetCommandPool()
	{
		m_commandPool.reset(new XVKCommandPool(*m_device, m_device->GraphicsQueueIndex(),
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
	}
	
	//----runtime----
	void ApplicationRZ::CreateSwapChain()
	{
		m_swapChain.reset(new XVKSwapChain(*m_device, m_presentMode));
		m_depthBuffer.reset(new XVKDepthBuffer(*m_commandPool, m_swapChain->GetExtent()));

		for (int i = 0; i < m_swapChain->GetImageViews().size(); i++)
		{
			m_imageAvailableSemaphores.emplace_back(*m_device);
			m_renderFinishedSemaphores.emplace_back(*m_device);
			m_inFlightFences.emplace_back(*m_device, true);
			m_uniformBuffers.emplace_back(*m_device);
		}

		m_graphicsPipeline.reset(new XVKGraphicsPipeline(*m_device, *m_swapChain, *m_depthBuffer,
			m_uniformBuffers, *m_scene));

		for (auto& imageView : m_swapChain->GetImageViews())
		{
			m_frameBuffers.emplace_back(*imageView, m_graphicsPipeline->GetRenderPass());
		}

		m_commandBuffers.reset(new XVKCommandBuffers(*m_commandPool, m_swapChain->GetImageViews().size()));
	}

	void ApplicationRZ::DeleteSwapChain()
	{
		m_commandBuffers.reset();
		m_frameBuffers.clear();
		m_graphicsPipeline.reset();
		m_uniformBuffers.clear();
		m_inFlightFences.clear();
		m_renderFinishedSemaphores.clear();
		m_imageAvailableSemaphores.clear();
		m_depthBuffer.reset();
		m_swapChain.reset();
	}

	void ApplicationRZ::DrawFrame()
	{
	
	}

	void ApplicationRZ::Render()
	{

	}

	void ApplicationRZ::Run()
	{

	}
}