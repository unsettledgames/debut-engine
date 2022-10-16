#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

/* NOTES
* - Apparently, ShapeSettings and Shapes are managed by bodies, so no need to clear their memory
* - INVESTIGATE: multiple colliders? Create body without shape is impossible

*/

using namespace JPH;

namespace JPH
{
	class Shape;
	class ShapeSettings;
	class JobSystem;
}

namespace Debut
{
	struct Physics3DSettings
	{
		uint32_t MaxBodies = 65536;
		uint32_t MaxPairs = 65536;
		uint32_t MaxContactConstraints = 10240;
		uint32_t NBodyMutexes = 0;

		uint32_t MaxJobs = 2048;
		uint32_t MaxPhysicsBarriers = 8;
		uint32_t MaxAllocatedSpace = 10 * 1024 * 1024;
	};

	namespace Layers
	{
		static constexpr uint8 NON_MOVING = 0;
		static constexpr uint8 MOVING = 1;
		static constexpr uint8 NUM_LAYERS = 2;
	};

	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	namespace BroadPhaseLayers
	{
		static constexpr BroadPhaseLayer NON_MOVING(0);
		static constexpr BroadPhaseLayer MOVING(1);
		static constexpr uint NUM_LAYERS(2);
	};

	class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual uint					GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
		{
			switch ((BroadPhaseLayer::Type)inLayer)
			{
			case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
	};


	class MyBodyActivationListener : public BodyActivationListener
	{
	public:
		virtual void		OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
		{
			//cout << "A body got activated" << endl;
		}

		virtual void		OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
		{
			//cout << "A body went to sleep" << endl;
		}
	};


	class MyContactListener : public ContactListener
	{
	public:
		// See: ContactListener
		virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, const CollideShapeResult& inCollisionResult) override
		{
			//cout << "Contact validate callback" << endl;

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
		{
			//cout << "A contact was added" << endl;
		}

		virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
		{
			//cout << "A contact was persisted" << endl;
		}

		virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
		{
			//cout << "A contact was removed" << endl;
		}
	};

	struct Rigidbody3DComponent;
	struct SphereCollider3DComponent;
	struct BoxCollider3DComponent;
	struct MeshCollider3DComponent;
	struct TransformComponent;

	class PhysicsMaterial3D;

	class PhysicsSystem3D
	{
	public:
		PhysicsSystem3D(const Physics3DSettings& settings);
		~PhysicsSystem3D();

		void Begin();
		void Step(float timestep);
		void End();

		void UpdateBody(TransformComponent& transform, Rigidbody3DComponent& body, BodyID bodyID);

		BodyID* CreateBoxColliderBody(const BoxCollider3DComponent& collider, const Rigidbody3DComponent& rb,
			const TransformComponent& transform);
		BodyID* CreateSphereColliderBody(const SphereCollider3DComponent& collider, const Rigidbody3DComponent& rb,
			const TransformComponent& transform);
		BodyID* CreateMeshColliderBody(const MeshCollider3DComponent& collider, const Rigidbody3DComponent& rb,
			const TransformComponent& transform);
		
		BodyID* CreateBody(Shape* shape, Ref<PhysicsMaterial3D> physicsMaterial, const Rigidbody3DComponent& rb,
			const Vec3& pos, const Quat& rot, EMotionType motionType, ObjectLayer layer);
		// Sphere
		// Mesh

		void AddBody(BodyID body);

	private:
		PhysicsSystem* m_PhysicsSystem;
		BPLayerInterfaceImpl* m_BPLayerInterface; 
		JobSystem* m_JobSystem;
		TempAllocatorImpl* m_TempAllocator;

		MyBodyActivationListener* m_BodyActivationListener;
		MyContactListener* m_ContactListener;

		std::vector<BodyID> m_BodyIDs;
		uint32_t m_NumCurrBodies = 0;
		bool m_Simulating = false;
	};
}