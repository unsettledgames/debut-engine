namespace Debut
{
    class Input
    {
        public static bool IsKeyDown(KeyCode key)
        {
            return Core.Input_IsKeyDown(key);
        }

        /*
        public static bool KeyPressed(KeyCode key)
        {
            return Core.Input_KeyPressed(key);
        }

        public static bool KeyReleased(KeyCode key)
        {
            return Core.Input_KeyReleased(key);
        }
        */
    }
}