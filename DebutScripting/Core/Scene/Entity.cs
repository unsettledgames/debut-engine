using System;

namespace Debut
{
    public class Entity
    {
        public readonly ulong ID;

        protected Entity() { ID = 0; }
        
        internal Entity(ulong id) { ID = id; }

        public int Id { get; set; }

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return Core.Entity_HasComponent(ID, componentType);
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			T component = new T() { Entity = this };
			return component;
		}

		public Entity FindEntityByName(string name)
		{
			ulong entityID = Core.Entity_FindEntityByName(name);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public T As<T>() where T : Entity, new()
		{
			object instance = Core.GetScriptInstance(ID);
			return instance as T;
		}
	}
}