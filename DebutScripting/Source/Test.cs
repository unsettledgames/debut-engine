using System;

public class Test
{
    public float FloatVar = -1;

    public Test()
    {
        FloatVar = 69420.0f;
    }

    public void SetFloat(float v) 
    { 
        FloatVar = v; 
    }
    public float GetFloat() 
    { 
        return FloatVar; 
    }

    public void Print() 
    { 
        Console.WriteLine("Hello C#! Float value: {0}", FloatVar);
    }
}