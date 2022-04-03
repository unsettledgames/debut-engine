#pragma once
#include "Core/Time.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/OrthographicCamera.h"
#include <glm/glm.hpp>

namespace Debut
{
	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotate = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void SetZoomLevel(float zoom) { m_ZoomLevel = zoom; CalculateView(); }
		void Resize(uint32_t width, uint32_t height);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }
		float GetZoomLevel() const { return m_ZoomLevel; }
		const OrthographicCameraBounds GetBounds() const { return m_Bounds; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizedEvent& e);
		void CalculateView();

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;		

		float m_CameraMovementSpeed = 2.0f;
		float m_CameraRotationSpeed = 40.0f;

		glm::vec3 m_CameraPosition = glm::vec3(0.0f);
		float m_CameraRotation = 0.0f;

		bool m_Rotate;
		OrthographicCameraBounds m_Bounds;
	};

}

