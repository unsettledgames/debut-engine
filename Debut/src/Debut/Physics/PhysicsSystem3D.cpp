#include <Debut/dbtpch.h>

#include <Debut/Scene/Components.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Physics/PhysicsMaterial3D.h>
#include <Debut/Physics/PhysicsSystem3D.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
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
		m_TempAllocator = new TempAllocatorImpl(settings.MaxAllocatedSpace);
		Factory::sInstance = new Factory();
		
		RegisterTypes();

		m_BodyIDs.resize(settings.MaxBodies);
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
		m_Simulating = true;
	}

	void PhysicsSystem3D::Step(float timestep)
	{
		m_PhysicsSystem->Update(timestep, std::max((1.0f / timestep) / 60.0f, 1.0f), 1, m_TempAllocator, m_JobSystem);
	}

	void PhysicsSystem3D::UpdateBody(TransformComponent& transform, Rigidbody3DComponent& body, BodyID bodyID)
	{
		if (std::find(m_BodyIDs.begin(), m_BodyIDs.end(), bodyID) == m_BodyIDs.end())
			return;
		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();
		Vec3 pos = bi.GetPosition(bodyID);
		Vec3 rotation = bi.GetRotation(bodyID).GetEulerAngles();

		transform.Translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
		transform.Rotation = { rotation.GetX(), rotation.GetY(), rotation.GetZ() };
	}

	void PhysicsSystem3D::End()
	{
		if (m_PhysicsSystem == nullptr)
			return;

		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();

		// Remove and destroy bodies
		for (uint32_t i = 0; i < m_NumCurrBodies; i++)
		{
			bi.RemoveBody(m_BodyIDs[i]);
			bi.DestroyBody(m_BodyIDs[i]);
		}

		m_Simulating = false;
	}

	BodyID* PhysicsSystem3D::CreateBoxColliderBody(const BoxCollider3DComponent& collider, const Rigidbody3DComponent& rb, 
		const TransformComponent& transform)
	{
		// Necessary data
		bool isStatic = rb.Type == Rigidbody3DComponent::BodyType::Static;
		glm::vec3 halfSize = (collider.Size / 2.0f) * transform.Scale;
		glm::vec3 offset = collider.Offset;
		glm::vec3 startPos = transform.Translation;
		glm::vec3 startRot = transform.Rotation;

		Ref<PhysicsMaterial3D> physicsMaterial = AssetManager::Request<PhysicsMaterial3D>(collider.Material);

		// Compute initial transform
		Vec3Arg pos = { startPos.x + offset.x, startPos.y + offset.y, startPos.z + offset.z };
		QuatArg rot = Quat::sEulerAngles({ startRot.x, startRot.y, startRot.z });

		// Body settings
		BodyInterface& bi = m_PhysicsSystem->GetBodyInterface();
		BoxShape* shape = new BoxShape({ halfSize.x, halfSize.y, halfSize.z });
		BodyCreationSettings bodySettings(shape, pos, rot, isStatic ? EMotionType::Static : EMotionType::Dynamic, isStatic ? Layers::NON_MOVING : Layers::MOVING);
		
		// Set mass
		bodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		bodySettings.mMassPropertiesOverride.mMass = rb.Mass;

		// Create body, set other properties
		Body* body = bi.CreateBody(bodySettings);
		bi.AddBody(body->GetID(), EActivation::Activate);
		bi.SetGravityFactor(body->GetID(), rb.GravityFactor);
		if (physicsMaterial != nullptr)
		{
			bi.SetFriction(body->GetID(), physicsMaterial->GetFriction());
			bi.SetRestitution(body->GetID(), physicsMaterial->GetRestitution());
		}

		m_BodyIDs[m_NumCurrBodies] = body->GetID();
		m_NumCurrBodies++;

		return &m_BodyIDs[m_NumCurrBodies - 1];
	}

	PhysicsSystem3D::~PhysicsSystem3D()
	{
		if (m_PhysicsSystem == nullptr)
			return;
		if (m_Simulating)
			End();

		delete Factory::sInstance;
		Factory::sInstance = nullptr;

		delete m_BodyActivationListener;
		delete m_ContactListener;
		delete m_BPLayerInterface;
		delete m_JobSystem;
		delete m_TempAllocator;
	}
}