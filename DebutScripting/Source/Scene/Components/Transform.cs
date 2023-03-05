namespace Debut
{
    public class Transform : Component
    {
        public Vector3 Translation
        {
            get
            {
                Core.TransformComponent_GetTranslation(Entity.ID, out Vector3 ret);
                return ret;
            }
            set
            {
                Core.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            { 
                Core.TransformComponent_GetEulerRotation(Entity.ID, out Vector3 ret);
                return ret;
            }
            set
            {
                Core.TransformComponent_SetEulerRotation(Entity.ID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                Core.TransformComponent_GetScale(Entity.ID, out Vector3 ret);
                return ret;
            }
            set
            {
                Core.TransformComponent_SetScale(Entity.ID, ref value);
            }
        }
    }
}