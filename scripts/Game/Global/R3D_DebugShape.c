class R3D_BaseDebugShape
{
	void Update(vector inTransform[4])
	{
		//m_Shape.SetMatrix(inTransform);
	}
};

class R3D_BaseDebugShapeT<Class T> : R3D_BaseDebugShape
{
	ref T m_Shape;
	vector m_Origin;
	
	void ~R3D_BaseDebugShapeT()
	{
		m_Shape = null;
	}
};

class R3D_DebugShape : R3D_BaseDebugShapeT<Shape>
{
	static R3D_DebugShape Create(ShapeType type, int color, ShapeFlags flags, vector p1, vector p2)
	{
		R3D_DebugShape res();
		res.m_Shape = Shape.Create(type, color, flags, p1, p2);
		return res;
	}

	static R3D_DebugShape CreateLines(int color, ShapeFlags flags, vector p[], int num)
	{
		R3D_DebugShape res();
		res.m_Shape = Shape.CreateLines(color, flags, p, num);
		return res;
	}

	static R3D_DebugShape CreateLine(int color, ShapeFlags flags, vector p0, vector p1)
	{
		vector line[2];
		line[0] = p0;
		line[1] = p1;
		return CreateLines(color, flags, line, 2);
	}

	static R3D_DebugShape CreateDirLine(int color, ShapeFlags flags, vector p, vector dir)
	{
		vector line[2];
		line[0] = p;
		line[1] = p + dir;
		return CreateLines(color, flags, line, 2);
	}

	static R3D_DebugShape CreateTris(int color, ShapeFlags flags, vector p[], int num)
	{
		R3D_DebugShape res();
		res.m_Shape = Shape.CreateTris(color, flags, p, num);
		return res;
	}

	static R3D_DebugShape CreateTriangle(int color, ShapeFlags flags, vector p0, vector p1, vector p2)
	{
		vector tri[3];
		tri[0] = p0;
		tri[1] = p1;
		tri[2] = p2;
		return CreateTris(color, flags, tri, 1);
	}

	static R3D_DebugShape CreateSphere(int color, ShapeFlags flags, vector origin, float radius)
	{
		R3D_DebugShape res();
		res.m_Origin = origin;
		res.m_Shape = Shape.CreateSphere(color, flags, "0 0 0", radius);
		return res;
	}

	static R3D_DebugShape CreateSphere(bool toggled, int colorA, int colorB, ShapeFlags flags, vector origin, float radius)
	{
		int color = colorA;
		if (toggled)
		{
			color = colorB;
		}

		return CreateSphere(color, flags, origin, radius);
	}
	
	static R3D_DebugShape CreateCylinder(int color, ShapeFlags flags, vector origin, float radius, float length)
	{
		R3D_DebugShape res();
		res.m_Origin = origin;
		res.m_Shape = Shape.CreateCylinder(color, flags, "0 0 0", radius, length);
		return res;
	}
	
	static R3D_DebugShape CreateArrow(vector from, vector to, float size, int color, ShapeFlags flags)
	{
		R3D_DebugShape res();
		res.m_Shape = Shape.CreateArrow(from, to, size, color, flags);
		return res;
	}

	override void Update(vector inTransform[4])
	{
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = m_Origin;
		
		Math3D.MatrixMultiply4(inTransform, transform, transform);
		m_Shape.SetMatrix(transform);
	}
};

class R3D_DebugText : R3D_BaseDebugShapeT<DebugTextWorldSpace>
{
	static BaseWorld WORLD;
	
	static R3D_DebugText Create(string text, vector origin, float size = 0.2)
	{
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		
		R3D_DebugText res();
		res.m_Origin = origin + Vector(0, size * 0.5, 0);
		res.m_Shape = DebugTextWorldSpace.CreateInWorld(WORLD, text, DebugTextFlags.FACE_CAMERA, transform, size);
		return res;
	}

	override void Update(vector inTransform[4])
	{
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = m_Origin;
		
		Math3D.MatrixMultiply4(inTransform, transform, transform);
		m_Shape.SetTransform(transform);
	}
};
