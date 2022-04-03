#pragma once
#include <ParticleSystem.h>
#include <Debut.h>


class Sandbox2D : public Debut::Layer
{
public:
	Sandbox2D() : Debut::Layer("Sandbox2D"), m_CameraController(1920.0f / 1080.0f, true), m_ParticleSystem(100000) {}
	virtual ~Sandbox2D() {}

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Debut::Timestep ts) override;
	virtual void OnEvent(Debut::Event& e) override;
	virtual void OnImGuiRender() override;

private:
	Debut::OrthographicCameraController m_CameraController;

	Debut::Ref<Debut::Texture2D> m_Texture;
	Debut::Ref<Debut::Texture2D> m_Checkerboard;
	Debut::Ref<Debut::SubTexture2D> m_BushTexture;

	Debut::Ref<Debut::FrameBuffer> m_FrameBuffer;
	glm::vec4 m_TriangleColor = glm::vec4(0.3, 0.3, 1, 1);

	ParticleSystem m_ParticleSystem;
	ParticleProps m_Particle;
};
