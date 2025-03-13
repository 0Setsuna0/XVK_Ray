#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan [0, 1] depth range, instead of OpenGL [-1, +1]
#define GLM_FORCE_RIGHT_HANDED // Vulkan has a left handed coordinate system (same as DirectX), OpenGL is right handed
#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace editor
{
	class Camera
	{
	public:

		void Reset(const glm::mat4& modelViewProjection);

		glm::mat4 GetViewMatrix() const;

		void SetProjection(float fov, float aspect, float nearPlane, float farPlane);

		bool OnKey(int key, int scancode, int action, int mods);
		bool OnCursorPosition(double xpos, double ypos);
		bool OnMouseButton(int button, int action, int mods);
		bool UpdateCamera(double speed, double timeDelta);

	private:

		void MoveForward(float d);
		void MoveRight(float d);
		void MoveUp(float d);
		void Rotate(float y, float x);
		void UpdateVectors();

		glm::vec4 position{};
		glm::vec4 front{ 0,0,-1,0 };
		glm::vec4 up{ 0,1,0,0 };
		glm::vec4 right{ 0,0,-1,0 };

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 rotation;

		bool cameraMovingLeft_{};
		bool cameraMovingRight_{};
		bool cameraMovingBackward_{};
		bool cameraMovingForward_{};
		bool cameraMovingDown_{};
		bool cameraMovingUp_{};

		float cameraRotX_{};
		float cameraRotY_{};

		double mousePosX_{};
		double mousePosY_{};

		bool mouseLeftPressed_{};
		bool mouseRightPressed_{};
	};
}