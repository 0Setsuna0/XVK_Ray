#include "UI.h"
#include "Vulkan/XVKDescriptorPool.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKDescriptorSetLayout.h"
#include "Vulkan/XVKFrameBuffer.h"
#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKRenderPass.h"
#include "Vulkan/XVKTransientCommand.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKWindow.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include <array>


void CheckVulkanResultCallback(const VkResult err)
{
	if (err != VK_SUCCESS)
	{
		std::runtime_error(std::string("ImGui Vulkan error"));
	}
}

namespace editor
{
	UI::UI(xvk::XVKCommandPool& commandPool,
		const xvk::XVKSwapChain& swapChain,
		const xvk::XVKDepthBuffer& depthBuffer,
		UserSettings& userSettings,
		Status& status)
		:userSettings(userSettings), status(status)
	{
		const auto& device = swapChain.GetDevice();
		const auto& window = device.Instance().GetWindow();

		std::vector<xvk::DescriptorBinding> descriptorBindings;
		descriptorBindings.emplace_back(0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);

		xvk_descriptorPool.reset(new xvk::XVKDescriptorPool(device, descriptorBindings, 1));
		xvk_renderPass.reset(new xvk::XVKRenderPass(device, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD,
			depthBuffer, swapChain));

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// Initialise ImGui GLFW adapter
		if (!ImGui_ImplGlfw_InitForVulkan(window.Handle(), true))
		{
			std::runtime_error("failed to initialise ImGui GLFW adapter");
		}

		// Initialise ImGui Vulkan adapter
		ImGui_ImplVulkan_InitInfo vulkanInit = {};
		vulkanInit.Instance = device.Instance().Handle();
		vulkanInit.PhysicalDevice = device.PhysicalHandle();
		vulkanInit.Device = device.Handle();
		vulkanInit.QueueFamily = device.GraphicsQueueIndex();
		vulkanInit.Queue = device.GraphicsQueue();
		vulkanInit.PipelineCache = nullptr;
		vulkanInit.DescriptorPool = xvk_descriptorPool->Handle();
		vulkanInit.RenderPass = xvk_renderPass->Handle();
		vulkanInit.MinImageCount = swapChain.GetMinImageCount();
		vulkanInit.ImageCount = static_cast<uint32_t>(swapChain.GetImages().size());
		vulkanInit.Allocator = nullptr;
		vulkanInit.CheckVkResultFn = CheckVulkanResultCallback;

		if (!ImGui_ImplVulkan_Init(&vulkanInit))
		{
			std::runtime_error("failed to initialise ImGui vulkan adapter");
		}

		auto& io = ImGui::GetIO();

		// No ini file.
		io.IniFilename = nullptr;

		// Window scaling and style.
		const auto scaleFactor = window.GetWindowScale();

		ImGui::StyleColorsDark();
		ImGui::GetStyle().ScaleAllSizes(scaleFactor);

		xvk::XVKTransientCommands::Submit(commandPool, [](VkCommandBuffer commandBuffer)
			{
				if (!ImGui_ImplVulkan_CreateFontsTexture())
				{
					std::runtime_error("failed to create ImGui font textures");
				}
			});

	}

	UI::~UI()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UI::Render(VkCommandBuffer commandBuffer, const xvk::XVKFrameBuffer& frameBuffer)
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();

		Draw();
		//ImGui::ShowStyleEditor();
		ImGui::Render();

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = xvk_renderPass->Handle();
		renderPassInfo.framebuffer = frameBuffer.Handle();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = xvk_renderPass->GetSwapChain().GetExtent();
		renderPassInfo.clearValueCount = 0;
		renderPassInfo.pClearValues = nullptr;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		vkCmdEndRenderPass(commandBuffer);
	}


	bool UI::WantsToCaptureKeyboard() const
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}
	bool UI::WantsToCaptureMouse() const
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	void UI::Draw()
	{
		if (!GetUserSettings().showSettings)
		{
			return;
		}

		const float distance = 10.0f;
		const ImVec2 pos = ImVec2(distance, distance);
		const ImVec2 posPivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(pos, ImGuiCond_Always, posPivot);

		const auto flags =
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings;

		if (ImGui::Begin("Settings", &GetUserSettings().showSettings, flags))
		{
			const auto& window = xvk_descriptorPool->GetDevice().Instance().GetWindow();
			ImGui::Text("Basic settings of this renderer");
			ImGui::Separator();

			ImGui::Text("Scene");
			ImGui::Separator();
			ImGui::Combo("SceneList", &GetUserSettings().sceneIndex, sceneList.data(), static_cast<int>(sceneList.size()));

			ImGui::Text("Statistics (%dx%d):", status.WindowSize.width, status.WindowSize.height);
			ImGui::Separator();
			ImGui::Text("Frame rate: %.1f fps", GetUserSettings().renderingMode == 2 ? status.FrameRate * 3: status.FrameRate);
			ImGui::Text("One frame cost:  %.1f ms", GetUserSettings().renderingMode == 2 ? status.OneFrameTimeCost / 3 : status.OneFrameTimeCost);

			ImGui::Text("Rendering Mode");
			ImGui::Separator();
			ImGui::Combo("Rendering Mode", &GetUserSettings().renderingMode, renderModes.data(), static_cast<int>(renderModes.size()));
			uint32_t minSpp = 1;
			uint32_t maxSpp = 100;
			uint32_t minBounces = 1;
			uint32_t maxBounces = 32;
			switch (GetUserSettings().renderingMode)
			{
			case 0: 
				//rasterization
				break;

			case 1: 
				//path tracing
				ImGui::Checkbox("Enable ray accumulation", &GetUserSettings().enableRayAccumulation);
				ImGui::SliderScalar("Spp", ImGuiDataType_U32, &GetUserSettings().spp, &minSpp, &maxSpp);
				ImGui::SliderScalar("Bounces", ImGuiDataType_U32, &GetUserSettings().numberOfBounces, &minBounces, &maxBounces);
				ImGui::Checkbox("Enable sky lighting", &GetUserSettings().enableSkyLighting);
				GetUserSettings().enableReuse = false;
				ImGui::NewLine();
				break;

			case 2: 
				// ReSTIR GI
				ImGui::Checkbox("Enable sky lighting", &GetUserSettings().enableSkyLighting);
				ImGui::SliderScalar("Bounces", ImGuiDataType_U32, &GetUserSettings().numberOfBounces, &minBounces, &maxBounces);
				GetUserSettings().spp = 6;
				GetUserSettings().numberOfBounces = 4;
				GetUserSettings().enableRayAccumulation = false;
				GetUserSettings().enableReuse = true;
				break;
			case 3:
				break;
			default:
				break;
			}
		}
		ImGui::End();
	}
}