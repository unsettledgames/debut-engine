#pragma once
#include "Core/Time.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/OrthographicCamera.h"
#include <glm/glm.hpp>

namespace Debut
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotate = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void SetZoomLevel(float zoom) { m_ZoomLevel = zoom; }

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }
		float GetZoomLevel() const { return m_ZoomLevel; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizedEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		float m_CameraMovementSpeed = 2.0f;
		float m_CameraRotationSpeed = 40.0f;

		glm::vec3 m_CameraPosition = glm::vec3(0.0f);
		float m_CameraRotation = 0.0f;

		bool m_Rotate;

		
	};

}

