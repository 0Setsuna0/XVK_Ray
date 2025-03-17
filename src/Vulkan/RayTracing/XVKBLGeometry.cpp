#include "XVKBLGeometry.h"
#include "Vulkan/XVKBuffer.h"
#include "Asset/Vertex.h"
#include "Asset/Scene.h"

namespace xvk::ray
{
	void XVKBLGeometry::AddGeometryTriangles(const vkAsset::AScene& scene,
		uint32_t vertexOffset,
		uint32_t vertexCount,
		uint32_t indexOffset,
		uint32_t indexCount,
		bool isOpaque)
	{
		VkAccelerationStructureGeometryKHR geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.pNext = nullptr;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.geometry.triangles.vertexData.deviceAddress = scene.GetVertexBuffer().GetDeviceAddress();
		geometry.geometry.triangles.vertexStride = sizeof(vkAsset::AVertex);
		geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		geometry.geometry.triangles.maxVertex = vertexCount;
		geometry.geometry.triangles.indexData.deviceAddress = scene.GetIndexBuffer().GetDeviceAddress();
		geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		geometry.geometry.triangles.transformData = {};//todo: add transform
		geometry.flags = isOpaque ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;

		VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo = {};
		buildOffsetInfo.firstVertex = vertexOffset / sizeof(vkAsset::AVertex);
		buildOffsetInfo.primitiveCount = indexCount / 3;
		buildOffsetInfo.primitiveOffset = indexOffset;
		buildOffsetInfo.transformOffset = 0;

		//add into the RayTracing pipeline from the scene's GPU buffers
		vk_geometry.push_back(geometry);
		vk_buildOffsetInfo.push_back(buildOffsetInfo);
	}
}