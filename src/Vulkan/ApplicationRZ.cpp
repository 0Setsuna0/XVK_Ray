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
	Application::Application(const WindowState& windowState, VkPresentModeKHR presentMode,
		bool enableValidationLayer)
		:m_presentMode(presentMode)
	{
		const auto validationLayers = enableValidationLayer ?
			std::vector<const char*>{"VK_LAYER_KHRONOS_validation"} : std::vector<const char*>{};
		
		m_window.reset(new XVKWindow(windowState));
		m_instance.reset(new XVKInstance(*m_window, validationLayers));
		std::vector<const char*> requiredExtension{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME 
		};
		SetDevice(requiredExtension);
		SetCommandPool();
	}

	Application::~Application()
	{
		DeleteSwapChain();
		m_commandPool.reset();
		m_device.reset();
		m_instance.reset();
		m_window.reset();
	}

	//handles
	const std::vector<VkExtensionProperties>& Application::GetExtensions() const
	{
		return m_instance->GetExtensions();
	}

	const std::vector<VkLayerProperties>& Application::GetLayers() const
	{
		return m_instance->GetLayers();
	}

	VkPhysicalDevice Application::GetPhysicalDevice() const
	{
		return m_device->PhysicalHandle();
	}

	//----init----
	void Application::SetDevice(const std::vector<const char*>& requiredExtensions)
	{
		m_device.reset(new XVKDevice(*m_instance, requiredExtensions));
	}

	void Application::SetCommandPool()
	{
		m_commandPool.reset(new XVKCommandPool(*m_device, m_device->GraphicsQueueIndex(),
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
	}
	
	//----runtime----
	void Application::CreateSwapChain()
	{
		m_swapChain.reset(new XVKSwapChain(*m_device, m_presentMode));
		m_depthBuffer.reset(new XVKDepthBuffer(*m_commandPool, m_swapChain->GetExtent()));

		m_imageAvailableSemaphores.reserve(m_swapChain->GetImageViews().size());
		m_renderFinishedSemaphores.reserve(m_swapChain->GetImageViews().size());
		m_inFlightFences.reserve(m_swapChain->GetImageViews().size());
		m_uniformBuffers.reserve(m_swapChain->GetImageViews().size());

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

	void Application::DeleteSwapChain()
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

	void Application::UpdateUniformBuffer()
	{
		m_uniformBuffers[currentFrame].SetValue(GetUniformBufferObject(m_swapChain->GetExtent()));
	}

	void Application::DrawFrame()
	{
		//synchronization
		auto& inFlightFence = m_inFlightFences[currentFrame];
		const VkSemaphore imageAvailableSemaphore = m_imageAvailableSemaphores[currentFrame].Handle();
		const VkSemaphore renderFinishedSemaphore = m_renderFinishedSemaphores[currentFrame].Handle();

		inFlightFence.Wait(std::numeric_limits<uint64_t>::max());

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_device->Handle(), m_swapChain->Handle(),
			std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if(result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_device->WaitIdle();
			DeleteSwapChain();
			CreateSwapChain();
			return;
		}
		else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			RUNTIME_ERROR("failed to acquire swap chain image!");
		}

		const auto commandBuffer = m_commandBuffers->Begin(currentFrame);
		Render(commandBuffer, currentFrame, imageIndex);
		m_commandBuffers->End(currentFrame);

		UpdateUniformBuffer();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer commandBuffers[] = { commandBuffer };
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffers;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		inFlightFence.Reset();
		VULKAN_RUNTIME_CHECK(vkQueueSubmit(m_device->GraphicsQueue(), 1, &submitInfo, inFlightFence.Handle()),
			"submit draw command buffer");

		VkSwapchainKHR swapChains[] = { m_swapChain->Handle() };
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(m_device->PresentQueue(), &presentInfo);
		if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_device->WaitIdle();
			DeleteSwapChain();
			CreateSwapChain();
			return;
		}
		else if(result != VK_SUCCESS)
		{
			RUNTIME_ERROR("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % m_swapChain->GetImageViews().size();
	}

	void Application::Render(VkCommandBuffer commandBuffer, const size_t currentFrame, const uint32_t imageIndex)
	{
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_graphicsPipeline->GetRenderPass().Handle();
		renderPassInfo.framebuffer = m_frameBuffers[imageIndex].Handle();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChain->GetExtent();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			VkDescriptorSet descriptorSets[] = { m_graphicsPipeline->GetDescriptorSet(currentFrame) };
			VkBuffer vertexBuffers[] = { m_scene->GetVertexBuffer().Handle() };
			VkBuffer indexBuffers[] = { m_scene->GetIndexBuffer().Handle() };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->Handle());
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline->GetPipelineLayout().Handle(),
				0, 1, descriptorSets, 0, nullptr);
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_scene->GetIndexBuffer().Handle(), 0, VK_INDEX_TYPE_UINT32);
		
			uint32_t vertexOffset = 0;
			uint32_t indexOffset = 0;
			for(const auto& model : m_scene->GetModels())
			{
				vkCmdDrawIndexed(commandBuffer, model->NumIndices(), 1, indexOffset, vertexOffset, 0);
				indexOffset += model->NumIndices();
				vertexOffset += model->NumVertices();
			}
		vkCmdEndRenderPass(commandBuffer);
	}

	void Application::Run()
	{
		currentFrame = 0;
		m_window->DrawFrame = [this]() { DrawFrame(); };
		m_window->OnKeyPressed = [this](const int key, const int scancode, const int action, const int mods) { OnKey(key, scancode, action, mods); };
		m_window->OnCursorPos = [this](const double xpos, const double ypos) { OnCursorPosition(xpos, ypos); };
		m_window->OnMouseButtonClicked = [this](const int button, const int action, const int mods) { OnMouseButton(button, action, mods); };
		m_window->OnScroll = [this](const double xoffset, const double yoffset) { OnScroll(xoffset, yoffset); };
		m_window->Run();
		m_device->WaitIdle();
	}
}