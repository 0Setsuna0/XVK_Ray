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
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
	{{0.5f, -0.5f, 0.0f}, {1.0f, 0.3f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

	{{ 0.5f, -0.5f, -0.5f }, {1.0f, 0.3f, 0.0f}, {1.0f, 0.0f} },
	{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

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

		xvk::WindowState windowState{ "test", 1920, 1080, true, false };
		
		PathTracer app(userSettings, windowState, VK_PRESENT_MODE_MAILBOX_KHR);

		vkAsset::AglTFModel model;
		model.loadglTFFile(ASSET_DIR"model/classroom/scene.gltf");
		std::vector<vkAsset::AglTFModel*> models = { &model };
		vkAsset::AScene scene(app.GetCommandPool(), models);
		std::vector<vkAsset::AScene*> sceneList = { &scene };
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
