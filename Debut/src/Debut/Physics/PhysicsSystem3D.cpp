#include <Debut/dbtpch.h>

#include <Debut/Physics/PhysicsSystem3D.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>

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

	PhysicsSystem3D::PhysicsSystem3D(const Physics3DSettings& settings)
	{
		RegisterDefaultAllocator();

		Factory::sInstance = new Factory();
		
		RegisterTypes();

		m_BodyActivationListener = new MyBodyActivationListener();
		m_ContactListener = new MyContactListener();

		TempAllocatorImpl tempAllocator(settings.MaxAllocatedSpace);
		JobSystemThreadPool jobSystem(settings.MaxJobs, settings.MaxPhysicsBarriers, thread::hardware_concurrency() - 1);

		m_PhysicsSystem = new PhysicsSystem();
		m_PhysicsSystem->Init(settings.MaxBodies, settings.NBodyMutexes, settings.MaxPairs, settings.MaxContactConstraints,
			m_BPLayerInterface, MyBroadPhaseCanCollide, MyObjectCanCollide);

		m_PhysicsSystem->SetBodyActivationListener(m_BodyActivationListener);
		m_PhysicsSystem->SetContactListener(m_ContactListener);
	}

	void PhysicsSystem3D::Begin()
	{
		m_PhysicsSystem->OptimizeBroadPhase();
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
	}
}