#include "Debut/dbtpch.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "OrthographicCameraController.h"

namespace Debut
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotate) : 
		m_AspectRatio(aspectRatio), m_Rotate(rotate),
		m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel),
		m_Bounds({ -aspectRatio * m_ZoomLevel, aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel })
	{

	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		DBT_PROFILE_FUNCTION();
		if (Input::IsKeyPressed(DBT_KEY_A))
			m_CameraPosition.x -= m_CameraMovementSpeed * ts;
		if (Input::IsKeyPressed(DBT_KEY_D))
			m_CameraPosition.x += m_CameraMovementSpeed * ts;

		if (Input::IsKeyPressed(DBT_KEY_S))
			m_CameraPosition.y -= m_CameraMovementSpeed * ts;
		if (Input::IsKeyPressed(DBT_KEY_W))
			m_CameraPosition.y += m_CameraMovementSpeed * ts;

		if (m_Rotate)
		{
			if (Input::IsKeyPressed(DBT_KEY_Q))
				m_CameraRotation -= m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(DBT_KEY_E))
				m_CameraRotation += m_CameraRotationSpeed * ts;
		}

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		DBT_PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<MouseScrolledEvent>(DBT_BIND(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizedEvent>(DBT_BIND(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel = e.GetOffsetY() > 0.0f ? m_ZoomLevel * 0.9f : m_ZoomLevel / 0.9f;
		m_CameraMovementSpeed = m_ZoomLevel * 2;

		CalculateView();
		return false;
	}

	void OrthographicCameraController::Resize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / height;
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizedEvent& e)
	{
		Resize(e.GetWidth(), e.GetHeight());
		return false;
	}

	void OrthographicCameraController::CalculateView()
	{
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
}