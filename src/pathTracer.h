#pragma once

#include "Vulkan/RayTracing/ApplicationRT.h"
#include "Asset/Model.h"
#include "Asset/Scene.h"
#include "Editor/UserSettings.h"
#include "Editor/Camera.h"

#include <memory>

class PathTracer final: public xvk::ray::ApplicationRT
{
public:
	PathTracer(const editor::UserSettings& userSettings, const xvk::WindowState& windowConfig, 
		VkPresentModeKHR presentMode);
	~PathTracer();

protected:
	void CreateSwapChain()override;
	void DeleteSwapChain()override;
	void DrawFrame()override;
	void Render(VkCommandBuffer commandBuffer, const size_t currentFrame, const uint32_t imageIndex)override;

	void OnKey(int key, int scancode, int action, int mods)override;
	void OnCursorPosition(double xpos, double ypos)override;
	void OnMouseButton(int button, int action, int mods) override;
	void OnScroll(double xoffset, double yoffset) override;
	vkAsset::UniformBufferObject GetUniformBufferObject(VkExtent2D extent) const override;

private:
	void PostTracerInit();
	void LoadCurrentScene(uint32_t sceneIndex);

	uint32_t renderedSceneIndex = 0;

	editor::UserSettings m_userSettings;
	editor::UserSettings m_userSettingsPrevious;
	editor::Camera m_camera;
	
	double lastTime = 0;
	double currentTime = 0;

	const std::vector<vkAsset::AScene> m_scenes;
	
	//ray tracing settings
	uint32_t spp;
	uint32_t totalSamples;
	bool rebuildRays;
};