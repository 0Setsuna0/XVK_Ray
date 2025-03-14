#pragma once
#include <vector>
#include <memory>
#include "XVKCommon.h"

namespace vkAsset 
{
	class AScene;
	class AUniformBuffer;
	class UniformBufferObject;
}

namespace xvk
{
	struct WindowState;
	class XVKWindow;
	class XVKInstance;
	class XVKSwapChain;
	class XVKDevice;
	class XVKDepthBuffer;
	class XVKGraphicsPipeline;
	class XVKFrameBuffer;
	class XVKCommandPool;
	class XVKCommandBuffers;
	class XVKSemaphore;
	class XVKFence;

	class Application
	{
	public:
		Application(const WindowState& windowState, VkPresentModeKHR presentMode,
			bool enableValidationLayer);
		virtual ~Application();
		
		const std::vector<VkExtensionProperties>& GetExtensions() const;
		const std::vector<VkLayerProperties>& GetLayers() const;
		VkPhysicalDevice GetPhysicalDevice() const;

		const XVKSwapChain& GetSwapChain() const { return *m_swapChain; }
		const XVKWindow& const_GetWindow() const { return *m_window; }
		const XVKDevice& GetDevice() const { return *m_device; }
		XVKWindow& GetWindow() { return *m_window; }
		XVKCommandPool& GetCommandPool() { return *m_commandPool; }

		void Run();

	protected:
		const XVKDepthBuffer& GetDepthBuffer() const { return *m_depthBuffer; }
		const std::vector<vkAsset::AUniformBuffer>& GetUniformBuffers() const { return m_uniformBuffers; }
		const XVKGraphicsPipeline& GetGraphicsPipeline() const { return *m_graphicsPipeline; }
		const XVKFrameBuffer& GetSwapChainFrameBuffer(uint32_t index) const { return m_frameBuffers[index]; }

		//override ui func
		virtual void CreateSwapChain();
		virtual void DeleteSwapChain();
		virtual void DrawFrame();
		virtual void Render(VkCommandBuffer commandBuffer, const size_t currentFrame, const uint32_t imageIndex);
		
		virtual void OnKey(int key, int scancode, int action, int mods) = 0;
		virtual void OnCursorPosition(double xpos, double ypos) = 0;
		virtual void OnMouseButton(int button, int action, int mods) = 0;
		virtual void OnScroll(double xoffset, double yoffset) = 0;

		virtual vkAsset::UniformBufferObject GetUniformBufferObject(VkExtent2D extent) const = 0;
		void UpdateUniformBuffer();
		const vkAsset::AScene* m_scene;

	private:
		void SetDevice(const std::vector<const char*>& requiredExtensions);
		void SetCommandPool();
		
		const VkPresentModeKHR m_presentMode;

		std::unique_ptr<XVKWindow> m_window;
		std::unique_ptr<XVKInstance> m_instance;
		std::unique_ptr<XVKDevice> m_device;
		std::unique_ptr<XVKSwapChain> m_swapChain;
		std::unique_ptr<XVKDepthBuffer> m_depthBuffer;
		std::vector<vkAsset::AUniformBuffer> m_uniformBuffers;
		std::unique_ptr<XVKGraphicsPipeline> m_graphicsPipeline;
		std::vector<XVKFrameBuffer> m_frameBuffers;
		std::unique_ptr<XVKCommandPool> m_commandPool;
		std::unique_ptr<XVKCommandBuffers> m_commandBuffers;
		std::vector<XVKSemaphore> m_imageAvailableSemaphores;
		std::vector<XVKSemaphore> m_renderFinishedSemaphores;
		std::vector<XVKFence> m_inFlightFences;

		uint32_t currentFrame;
	};
}