#include <Debut/dbtpch.h>

#include <Debut/Scene/Components.h>
#include <Debut/Physics/PhysicsSystem3D.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace JPH;

namespace Debut
{
	// Function that determines if two object layers can collide
	static bool MyObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	};
	// Function that determines if two broadphase layers can collide
	static bool MyBroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}

	// Callback for traces, connect this to your own trace function if you have one
	static void TraceImpl(const char* inFMT, ...)
	{
		// Format the message
		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		std::string format(inFMT);
		Log.CoreInfo("{0}", list);
	}

	PhysicsSystem3D::PhysicsSystem3D(const Physics3DSettings& settings)
	{
		Trace = TraceImpl;
		Factory::sInstance = new Factory();
		
		RegisterTypes();

		m_JobSystem = new JobSystemThreadPool(settings.MaxJobs, settings.MaxPhysicsBarriers, thread::hardware_concurrency() - 1);
		
		m_BPLayerInterface = new BPLayerInterfaceImpl();
		m_BodyActivationListener = new MyBodyActivationListener();
		m_ContactListener = new MyContactListener();

		m_PhysicsSystem = new PhysicsSystem();
		m_PhysicsSystem->Init(settings.MaxBodies, settings.NBodyMutexes, settings.MaxPairs, settings.MaxContactConstraints,
			*m_BPLayerInterface, MyBroadPhaseCanCollide, MyObjectCanCollide);

		m_PhysicsSystem->SetBodyActivationListener(m_BodyActivationListener);
		m_PhysicsSystem->SetContactListener(m_ContactListener);
	}

	void PhysicsSystem3D::AddBody(BodyID body)
	{
		auto& bi = m_PhysicsSystem->GetBodyInterface();
		bi.AddBody(body, EActivation::Activate);
	}

	void PhysicsSystem3D::Begin()
	{
		m_PhysicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem3D::Step(float timestep)
	{
		m_PhysicsSystem->Update(timestep, (1.0f / timestep) / 60.0, 1, nullptr, m_JobSystem);
	}

	void PhysicsSystem3D::UpdateBody(Rigidbody3DComponent& body, BodyID bodyID)
	{
		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();
		Vec3 pos = bi.GetPosition(bodyID);

		body.Position = { pos.GetX(), pos.GetY(), pos.GetZ() };
	}

	void PhysicsSystem3D::End()
	{
		if (m_PhysicsSystem == nullptr)
			return;

		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();

		// Remove and destroy bodies
		for (uint32_t i = 0; i < m_BodyIDs.size(); i++)
		{
			bi.RemoveBody(m_BodyIDs[i]);
			bi.DestroyBody(m_BodyIDs[i]);
		}
	}

	BodyID PhysicsSystem3D::CreateBoxColliderBody(const glm::vec3& size, const glm::vec3& offset)
	{
		glm::vec3 halfSize = (size / 2.0f);
		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();
		BoxShapeSettings* boxSettings = new BoxShapeSettings({ halfSize.x, halfSize.y, halfSize.z });
		BodyCreationSettings bodySettings(boxSettings, { offset.x, offset.y, offset.z }, QuatArg::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

		auto result = bodySettings.ConvertShapeSettings();
		if (result.HasError())
		{
			std::string error = bodySettings.ConvertShapeSettings().GetError();
			Log.CoreInfo("Shape creation: {0}", error);
		}
		
		return bi.CreateAndAddBody(bodySettings, EActivation::Activate);
	}

	PhysicsSystem3D::~PhysicsSystem3D()
	{
		if (m_PhysicsSystem == nullptr)
			return;
		End();

		delete Factory::sInstance;
		Factory::sInstance = nullptr;

		delete m_BodyActivationListener;
		delete m_ContactListener;
		delete m_BPLayerInterface;
		delete m_JobSystem;
	}
}