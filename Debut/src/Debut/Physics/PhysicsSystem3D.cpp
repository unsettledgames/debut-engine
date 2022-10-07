#include <Debut/dbtpch.h>

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
		Log.CoreInfo(inFMT, list);
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

	Body* PhysicsSystem3D::CreateBody()
	{
		BoxShapeSettings* boxSettings = new BoxShapeSettings(Vec3(0.0f, 0.0f, 0.0f));
		BodyCreationSettings bodySettings(boxSettings, Vec3(0.0f, 0.0f, 0.0f), QuatArg::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

		auto& bi = m_PhysicsSystem->GetBodyInterface();
		return bi.CreateBody(bodySettings);
	}

	void PhysicsSystem3D::AddBody(BodyID body)
	{
		auto& bi = m_PhysicsSystem->GetBodyInterface();
		bi.AddBody(body, EActivation::Activate);
	}

	void PhysicsSystem3D::Begin()
	{
		auto& bi = m_PhysicsSystem->GetBodyInterface();

		BoxShapeSettings* boxSettings = new BoxShapeSettings(Vec3(1.0f, 1.0f, 1.0f));
		BodyCreationSettings bodySettings(boxSettings, Vec3(0.0f, 0.0f, 0.0f), QuatArg::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		
		auto body = bi.CreateBody(bodySettings);
		bi.AddBody(body->GetID(), EActivation::DontActivate);

		m_PhysicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem3D::Step(float timestep)
	{
		m_PhysicsSystem->Update(timestep, (1.0f / timestep) / 60.0, 1, nullptr, m_JobSystem);
	}

	void PhysicsSystem3D::UpdateBody(Rigidbody3DComponent& body, BodyID bodyID)
	{
	}

	void PhysicsSystem3D::End()
	{
		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();

		// Remove and destroy bodies
		for (uint32_t i = 0; i < m_BodyIDs.size(); i++)
		{
			bi.RemoveBody(m_BodyIDs[i]);
			bi.DestroyBody(m_BodyIDs[i]);
		}
	}

	PhysicsSystem3D::~PhysicsSystem3D()
	{
		End();

		delete Factory::sInstance;
		Factory::sInstance = nullptr;

		delete m_BodyActivationListener;
		delete m_ContactListener;
		delete m_BPLayerInterface;
		delete m_JobSystem;
	}
}