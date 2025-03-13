#include "Camera.h"
#include "Vulkan/XVKCommon.h"
namespace editor
{
	void Camera::Reset(const glm::mat4& modelView)
	{
		const auto inverse = glm::inverse(modelView);

		position = inverse * glm::vec4(0, 0, 0, 1);
		rotation = glm::mat4(glm::mat3(modelView));

		cameraRotX_ = 0;
		cameraRotY_ = 0;

		mouseLeftPressed_ = false;
		mouseRightPressed_ = false;

		UpdateVectors();
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		const auto model = glm::mat4(1);
		const auto view = rotation * glm::translate(glm::mat4(1), -glm::vec3(position));

		return view * model;
	}

	void Camera::SetProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		projectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
	}

	void Camera::MoveForward(float d)
	{
		position += front * d;
	}

	void Camera::MoveRight(float d)
	{
		position += right * d;
	}

	void Camera::MoveUp(float d)
	{
		position += up * d;
	}

	void Camera::Rotate(float y, float x)
	{
		rotation =
			glm::rotate(glm::mat4(1), x, glm::vec3(1, 0, 0)) *
			rotation *
			glm::rotate(glm::mat4(1), y, glm::vec3(0, 1, 0));

		UpdateVectors();
	}

	void Camera::UpdateVectors()
	{
		const auto inverse = glm::inverse(rotation);

		right = inverse * glm::vec4(1, 0, 0, 0);
		up = inverse * glm::vec4(0, 1, 0, 0);
		front = inverse * glm::vec4(0, 0, -1, 0);
	}

	bool Camera::OnKey(int key, int scancode, int action, int mods)
	{
		switch (key)
		{
		case GLFW_KEY_S: cameraMovingBackward_ = action != GLFW_RELEASE; return true;
		case GLFW_KEY_W: cameraMovingForward_ = action != GLFW_RELEASE; return true;
		case GLFW_KEY_A: cameraMovingLeft_ = action != GLFW_RELEASE; return true;
		case GLFW_KEY_D: cameraMovingRight_ = action != GLFW_RELEASE; return true;
		case GLFW_KEY_Q: cameraMovingDown_ = action != GLFW_RELEASE; return true;
		case GLFW_KEY_E: cameraMovingUp_ = action != GLFW_RELEASE; return true;
		default: return false;
		}
	}
	bool Camera::OnCursorPosition(double xpos, double ypos)
	{
		auto x_offset = static_cast<float>(xpos - mousePosX_);
		auto y_offset = static_cast<float>(ypos - mousePosY_);

		if (mouseLeftPressed_)
		{
			cameraRotX_ += x_offset;
			cameraRotY_ += y_offset;
		}

		mousePosX_ = xpos;
		mousePosY_ = ypos;

		return mouseLeftPressed_;
	}
	bool Camera::OnMouseButton(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			mouseLeftPressed_ = action == GLFW_PRESS;
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			mouseRightPressed_ = action == GLFW_PRESS;
		}

		return true;
	}
	bool Camera::UpdateCamera(double speed, double deltaTime)
	{
		const auto d = static_cast<float>(speed * deltaTime);

		if (cameraMovingLeft_) MoveRight(-d);
		if (cameraMovingRight_) MoveRight(d);
		if (cameraMovingBackward_) MoveForward(-d);
		if (cameraMovingForward_) MoveForward(d);
		if (cameraMovingDown_) MoveUp(-d);
		if (cameraMovingUp_) MoveUp(d);

		const float sense = 0.005;
		Rotate(cameraRotX_ * sense, cameraRotY_ * sense);

		const bool updated =
			cameraMovingLeft_ ||
			cameraMovingRight_ ||
			cameraMovingBackward_ ||
			cameraMovingForward_ ||
			cameraMovingDown_ ||
			cameraMovingUp_ ||
			cameraRotY_ != 0 ||
			cameraRotX_ != 0;

		cameraRotY_ = 0;
		cameraRotX_ = 0;

		return updated;
	}
}