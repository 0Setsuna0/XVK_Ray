#include "pathTracer.h"
#include "Editor/Camera.h"
#include "Editor/UserSettings.h"
#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKWindow.h"
#include "Vulkan/XVKFrameBuffer.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKCommandBuffers.h"
#include "Asset/Model.h"
#include "Asset/Scene.h"	
#include "Asset/Texture.h"
#include "Asset/UniformBuffer.h"
#include <algorithm>
const bool EnableValidationLayers =
#ifdef NDEBUG
	false;
#else
	true;
#endif

PathTracer::PathTracer(const editor::UserSettings& userSettings, const xvk::WindowState& windowState,
	VkPresentModeKHR presentMode)
	:ApplicationRT(windowState, presentMode, EnableValidationLayers),
	m_userSettings(userSettings)
{
	
}

PathTracer::~PathTracer()
{

}

void PathTracer::PostTracerInit(const std::vector<vkAsset::AScene*>& sceneList)
{
	m_scenes = sceneList;
	LoadCurrentScene(m_userSettings.sceneIndex);
	CreateAccelerationStructures();
	CreateSwapChain();
}

void PathTracer::CreateSwapChain()
{
	xvk::ray::ApplicationRT::CreateSwapChain();
	ui.reset(new editor::UI(GetCommandPool(), GetSwapChain(), GetDepthBuffer(), m_userSettings));
	rebuildRays = true;
}

void PathTracer::DeleteSwapChain()
{
	xvk::ray::ApplicationRT::DeleteSwapChain();
}

void PathTracer::DrawFrame()
{
	if (renderedSceneIndex != m_userSettings.sceneIndex)
	{
		GetDevice().WaitIdle();
		DeleteSwapChain();
		DeleteAccelerationStructures();
		LoadCurrentScene(m_userSettings.sceneIndex);
		renderedSceneIndex = m_userSettings.sceneIndex;
		CreateAccelerationStructures();
		CreateSwapChain();
		return;
	}

	if (rebuildRays || !m_userSettings.enableRayAccumulation)
	{
		totalSamples = 0;
		rebuildRays = false;
	}

	spp = glm::clamp(m_userSettings.maxSpp - totalSamples, 0u, m_userSettings.spp);
	totalSamples += spp;

	xvk::Application::DrawFrame();
}
void PathTracer::Render(VkCommandBuffer commandBuffer, const size_t currentFrame, const uint32_t imageIndex)
{
	lastTime = currentTime;
	currentTime = GetWindow().GetTime();
	const float deltaTime = static_cast<float>(currentTime - lastTime);

	rebuildRays = m_camera.UpdateCamera(1, deltaTime);

	//todo: add path tracing code here
	m_userSettings.enableRayTracing ? xvk::ray::ApplicationRT::Render(commandBuffer, currentFrame, imageIndex) :
		xvk::Application::Render(commandBuffer, currentFrame, imageIndex);
	ui->Render(commandBuffer, GetSwapChainFrameBuffer(currentFrame));
}

void PathTracer::OnKey(int key, int scancode, int action, int mods)
{
	if (ui->WantsToCaptureKeyboard())
	{
		return;
	}
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			GetWindow().Close();
		}
	}

	rebuildRays = m_camera.OnKey(key, scancode, action, mods);
}

void PathTracer::OnCursorPosition(double xpos, double ypos)
{
	if (ui->WantsToCaptureMouse())
	{
		return;
	}
	rebuildRays = m_camera.OnCursorPosition(xpos, ypos);
}

void PathTracer::OnMouseButton(int button, int action, int mods)
{
	if (ui->WantsToCaptureMouse())
	{
		return;
	}
	rebuildRays = m_camera.OnMouseButton(button, action, mods);
}

void PathTracer::OnScroll(double xoffset, double yoffset)
{
	auto prevFov = m_userSettings.fov;
	m_userSettings.fov = std::clamp(m_userSettings.fov - static_cast<float>(yoffset), 
		editor::UserSettings::fovMin, editor::UserSettings::fovMax);
	//if fov changed, rebuild rays
	rebuildRays = prevFov != m_userSettings.fov;
}

void PathTracer::LoadCurrentScene(uint32_t sceneIndex)
{
	m_scene = m_scenes[sceneIndex];
	m_camera.Reset(m_scene->GetDefaultModelViewMatrix());
	rebuildRays = true;
}

vkAsset::UniformBufferObject PathTracer::GetUniformBufferObject(VkExtent2D extent)const
{
	vkAsset::UniformBufferObject ubo{};
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 model2 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//ubo.modelView = m_camera.GetViewMatrix() * model2 * model;
	ubo.modelView = m_camera.GetViewMatrix();
	ubo.projection = glm::perspective(glm::radians(m_userSettings.fov), 
		static_cast<float>(extent.width) / static_cast<float>(extent.height), 0.001f, 1000.0f);
	ubo.projection[1][1] *= -1;//flip y axis to fit vulkan
	ubo.modeViewInverse = glm::inverse(ubo.modelView);
	ubo.projectionInverse = glm::inverse(ubo.projection);
	ubo.spp = spp;
	ubo.totalNumberOfSamples = totalSamples;
	ubo.numberOfBounces = m_userSettings.numberOfBounces;
	ubo.randomSeed = 1;
	ubo.hasSkyBox = m_userSettings.enableSkyLighting;

	return ubo;
}