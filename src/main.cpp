#include <Vulkan/XVKBuffer.h>
#include "Vulkan/XVKInstance.h"
#include "Vulkan/XVKWindow.h"
#include "Vulkan/XVKDevice.h"
#include "Vulkan/XVKSwapChain.h"
#include "Vulkan/XVKRenderPass.h"
#include "Vulkan/XVKDescriptorSetLayout.h"
#include "Vulkan/XVKCommandPool.h"
#include "Vulkan/XVKCommandBuffers.h"
#include "Vulkan/XVKDepthBuffer.h"
#include "Vulkan/XVKDescriptorSetManager.h"
#include "Vulkan/XVKDescriptorSet.h"
#include "Vulkan/XVKShaderModule.h"
#include "Vulkan/XVKPipelineLayout.h"
#include "Vulkan/XVKFrameBuffer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cstdlib>
#include "Vulkan/XVKSemaphore.h"
#include "Vulkan/XVKFence.h"
#include "Vulkan/XVKImage.h"
#include "Vulkan/XVKImageView.h"
#include "Asset/TextureImage.h"
#include "Asset/Texture.h"
#include "Asset/Scene.h"
#include "Asset/Model.h"
#include "Vulkan/XVKDeviceMemory.h"
#include "Utility/BufferUtil.h"
#include "pathTracer.h"
#include "Editor/UserSettings.h"
#include "Editor/Camera.h"
const int MAX_FRAMES_IN_FLIGHT = 3;

int main()
{
	try
	{
		editor::UserSettings userSettings{};
		userSettings.sceneIndex = 0;
		userSettings.enableRayAccumulation = false;
		userSettings.enableRayTracing = false;
		userSettings.fov = 45.0f;
		userSettings.spp = 12;
		userSettings.numberOfBounces = 10;
		userSettings.maxSpp = 1000;
		userSettings.showSettings = true;
		userSettings.enableSkyLighting = false;

		xvk::WindowState windowState{ "XVKRay", 1920, 1080, true, false };
		
		PathTracer app(userSettings, windowState, VK_PRESENT_MODE_MAILBOX_KHR);

		vkAsset::AglTFModel model;
		model.loadglTFFile(ASSET_DIR"model/sb2/scene.gltf");
		vkAsset::AglTFModel model1;
		model1.loadglTFFile(ASSET_DIR"model/cb2/scene.gltf");
		vkAsset::AglTFModel model2;
		model2.loadglTFFile(ASSET_DIR"model/white_living/scene.gltf");
		std::vector<vkAsset::AglTFModel*> models = { &model };
		std::vector<vkAsset::AglTFModel*> models1 = { &model1 };
		std::vector<vkAsset::AglTFModel*> models2 = { &model2 };
		vkAsset::AScene scene(app.GetCommandPool(), models);
		vkAsset::AScene scene1(app.GetCommandPool(), models1);
		vkAsset::AScene scene2(app.GetCommandPool(), models2);
		std::vector<vkAsset::AScene*> sceneList = { &scene, &scene1, &scene2 };
		app.PostTracerInit(sceneList);
		app.Run();

	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
