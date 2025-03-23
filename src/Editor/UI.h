#pragma once
#include "Vulkan/XVKCommon.h"
#include "UserSettings.h"
#include <memory>
namespace xvk
{
	class XVKCommandPool;
	class XVKDepthBuffer;
	class XVKDescriptorPool;
	class XVKFrameBuffer;
	class XVKRenderPass;
	class XVKSwapChain;
}

namespace editor
{
	class UI
	{
	public:
		UI(xvk::XVKCommandPool& commandPool,
			const xvk::XVKSwapChain& swapChain,
			const xvk::XVKDepthBuffer& depthBuffer,
			UserSettings& userSettings);
		~UI();

		void Render(VkCommandBuffer commandBuffer, const xvk::XVKFrameBuffer& frameBuffer);

		bool WantsToCaptureKeyboard() const;
		bool WantsToCaptureMouse() const;

		UserSettings& GetUserSettings() { return userSettings; }

	private:

		void Draw();

		std::unique_ptr<xvk::XVKDescriptorPool> xvk_descriptorPool;
		std::unique_ptr<xvk::XVKRenderPass> xvk_renderPass;
		UserSettings& userSettings;
	};
}