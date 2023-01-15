namespace Debut
{
    public struct Vector2
    {
        public float x;
        public float y;

        public static Vector2 Zero = new Vector2(0.0f);

        public Vector2(float scalar)
        {
            x = scalar;
            y = scalar;
        }

        public Vector2(float x, float y)
        {
            this.x = x; 
            this.y = y;
        }

        // OPERATORS
        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x + b.x, a.y + b.y);
        }

        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x - b.x, a.y - b.y);
        }

        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x * scalar, vector.y * scalar);
        }

        public static Vector2 operator /(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x / scalar, vector.y / scalar);
        }
    }
}