#pragma once

#include "Vulkan/RayTracing/ApplicationRT.h"
#include "Asset/Model.h"
#include "Asset/Scene.h"
#include "Editor/UserSettings.h"
#include "Editor/Camera.h"

#include <memory>

class PathTracer : public xvk::ray::ApplicationRT
{
public:
	PathTracer(const editor::UserSettings& userSettings, const xvk::WindowState& windowConfig, 
		VkPresentModeKHR presentMode);
	~PathTracer() { ApplicationRT::test = 0; }
private:
	uint32_t renderedSceneIndex = 0;
	editor::UserSettings m_userSettings;
	//std::unique_ptr<editor::Camera> m_camera;
	std::vector<vkAsset::AScene> m_scenes;

};