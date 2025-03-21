#include "Model.h"
#include <glm/gtc/type_ptr.hpp>

namespace vkAsset
{
	AglTFModel::AglTFModel()
	{

	}

	AglTFModel::~AglTFModel()
	{
		for (auto node : nodes)
		{
			delete node;
		}
	}

	void AglTFModel::loadImages(tinygltf::Model& input)
	{
		images.resize(input.images.size());
		for (int i = 0; i < input.images.size(); i++)
		{
			tinygltf::Image& glTFImage = input.images[i];
			images[i].width = glTFImage.width;
			images[i].height = glTFImage.height;

			unsigned char* buffer = nullptr;
			VkDeviceSize bufferSize = 0;

			if (glTFImage.component == 3)
			{
				bufferSize = glTFImage.width * glTFImage.height * 4;
				buffer = new unsigned char[bufferSize];
				unsigned char* rgba = buffer;
				unsigned char* rgb = &glTFImage.image[0];
				for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i)
				{
					memcpy(rgba, rgb, sizeof(unsigned char) * 3);
					rgba[3] = 255;
					rgba += 4;
					rgb += 3;
				}
			}
			else
			{
				bufferSize = glTFImage.image.size();
				buffer = new unsigned char[bufferSize];
				memcpy(buffer, glTFImage.image.data(), bufferSize); 
			}


			images[i].bufferData = buffer;
		}
	}

	void AglTFModel::loadTextures(tinygltf::Model& input)
	{
		textures.resize(input.textures.size());
		for (int i = 0; i < input.textures.size(); i++)
		{
			textures[i].imageIndex = input.textures[i].source;
		}
	}

	void AglTFModel::loadMaterials(tinygltf::Model& input)
	{
		materials.resize(input.materials.size());
		for (int i = 0; i < input.materials.size(); i++)
		{
			tinygltf::Material glTFMaterial = input.materials[i];
			// Get the base color factor
			if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
				materials[i].baseColor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
			}
			// Get base color texture index
			if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
				materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
			}
			else
			{
				materials[i].baseColorTextureIndex = -1;
			}
			materials[i].materialModel = AMaterial::Enum::Lambertian;
		}
	}

	void AglTFModel::loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, AglTFModel::Node* parent,
		std::vector<uint32_t>& indexBuffer, std::vector<AVertex>& vertexBuffer)
	{
		AglTFModel::Node* node = new AglTFModel::Node{};
		node->matrix = glm::mat4(1.0f);
		node->parent = parent;

		if (inputNode.translation.size() == 3) 
		{
			node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}
		if (inputNode.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->matrix *= glm::mat4(q);
		}
		if (inputNode.scale.size() == 3)
		{
			node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}
		if (inputNode.matrix.size() == 16) 
		{
			node->matrix = glm::make_mat4x4(inputNode.matrix.data());
		};

		// Load node's children
		if (inputNode.children.size() > 0) 
		{
			for (size_t i = 0; i < inputNode.children.size(); i++) 
			{
				loadNode(input.nodes[inputNode.children[i]], input, node, indexBuffer, vertexBuffer);
			}
		}

		if (inputNode.mesh > -1)
		{
			const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;
				// Vertices
				{
					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					size_t vertexCount = 0;

					// Get buffer data for vertex positions
					if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}
					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					// Get buffer data for vertex texture coordinates
					// glTF supports multiple sets, we only load the first one
					if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
						texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}

					// Append data to model's vertex buffer
					for (size_t v = 0; v < vertexCount; v++) {
						AVertex vert{};
						vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
						vert.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
						//vert.color = glm::vec3(1.0f);
						vert.materialIndex = glTFPrimitive.material;
						vertexBuffer.push_back(vert);
					}
				}
				// Indices
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
					const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				Primitive primitive{};
				primitive.firstIndex = firstIndex;
				primitive.indexCount = indexCount;
				primitive.materialIndex = glTFPrimitive.material;
				node->mesh.primitives.push_back(primitive);
			}
		}

		if (parent)
		{
			parent->children.push_back(node);
		}
		else 
		{
			nodes.push_back(node);
		}
	}

	void AglTFModel::loadglTFFile(std::string filePath)
	{
		tinygltf::Model glTFInput;
		tinygltf::TinyGLTF glTFContext;
		std::string error, warning;

		bool fileLoaded = glTFContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filePath);

		if (fileLoaded)
		{
			loadImages(glTFInput);
			loadTextures(glTFInput);
			loadMaterials(glTFInput);
			const tinygltf::Scene& scene = glTFInput.scenes[0];
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
				loadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
			}
		}
		else
		{
			throw std::runtime_error("Fail to load glTF file");
		}
	}
}