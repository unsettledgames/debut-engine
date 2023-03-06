using System;
using Debut;

public class Test : Entity
{
    public float FloatVar = -1;

    private Transform m_Transform;

    void OnStart()
    {
        Console.WriteLine("OnStart called");
        m_Transform = GetComponent<Transform>();
    }

    void OnUpdate(float ts)
    {
        Vector3 translation = Vector3.Zero;
        Vector3 rotation = Vector3.Zero;
        Vector3 scale = Vector3.Zero;
        bool update = false;

        if (Input.IsKeyDown(KeyCode.DBT_KEY_A))
        {
            translation.x -= ts;
            update = true;
        }
        if (Input.IsKeyDown(KeyCode.DBT_KEY_D))
        {
            translation.x += ts;
            update = true;
        }
        if (Input.IsKeyDown(KeyCode.DBT_KEY_W))
        {
            translation.y += ts;
            update = true;
        }
        if (Input.IsKeyDown(KeyCode.DBT_KEY_S))
        {
            translation.y -= ts;
            update = true;
        }

        if (update)
        {
            m_Transform.LocalTranslation += translation * 2.0f;
            m_Transform.Rotation = m_Transform.Rotation + rotation * 2.0f;
            m_Transform.Scale = m_Transform.Scale + scale;
        }
    }

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

