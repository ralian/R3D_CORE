class R3D_BuoyancyComponentClass : R3D_VehiclePartBaseComponentClass
{
	//! REMEMBER: Keep same with 'R3D_BuoyancyComponent'
	static const int MAX_POINTS = 128;
	static const int MAX_INDICES = 1024;
	static const int MAX_PARTICLES = 2048;

	[Attribute(params: "obj")]
	ResourceName m_Model;

	[Attribute()]
	ref PointInfo m_Position;

	[Attribute()]
	vector m_Size;

	int m_NumPoints;
	int m_NumIndices;
		
	vector m_Points[MAX_POINTS];
	int m_Indices[MAX_INDICES];

	float m_Volume;
	float m_Area;

	void R3D_BuoyancyComponentClass(BaseContainer prefab)
	{
		Load();
	}

	void Load()
	{
		Print("Loading mesh!");

		m_NumPoints = 0;
		m_NumIndices = 0;
		m_Volume = 0;
		m_Area = 0;

		string path = m_Model.GetPath();

		bool success = false;

		if (path != string.Empty)
		{
			Print("Loading from mesh");
			success = CreateBodyFromMesh(path);
		}

		if (!success)
		{
			Print("Loading from bounding box");
			success = CreateFromBoundingBox();
		}
		
		if (!success)
		{
			Print("Failed to load mesh");
			return;
		}

		for (int i = 0; i < m_NumIndices; i += 3)
		{
			int i1 = m_Indices[i + 0];
			int i2 = m_Indices[i + 1];
			int i3 = m_Indices[i + 2];

			vector v1 = m_Points[i1];
			vector v2 = m_Points[i2];
			vector v3 = m_Points[i3];

			vector a = v2 - v1;
			vector b = v3 - v1;

			m_Volume += vector.Dot(a * b, v1) / 6.0;
			m_Area += (a * b).Length() * 0.5;
			
			vector normal = ((v2 - v1) * (v3 - v1)).Normalized();
			if (vector.Dot(normal, v1) < 0)
			{
				//m_Indices[i + 0] = i1;
				//m_Indices[i + 1] = i3;
				//m_Indices[i + 2] = i2;
			}
		}
	}

	bool CreateBodyFromMesh(string path)
	{
		vector position;

		if (m_Position)
		{
			vector transform[4];
			m_Position.GetTransform(transform);
			position = transform[3];
		}

		m_NumPoints = 0;
		m_NumIndices = 0;
		
		if (!FileIO.FileExists(path))
		{
			return false;
		}
		
		FileHandle file = FileIO.OpenFile(path, FileMode.READ);
		
		string line;
		while (file.ReadLine(line) >= 0)
		{
			TStringArray tokens = new TStringArray;
			line.Split(" ", tokens, true);
			
			if (tokens[0] == "v")
			{
				float x = tokens[1].ToFloat() * m_Size[0];
				float y = tokens[2].ToFloat() * m_Size[1];
				float z = tokens[3].ToFloat() * m_Size[2];
				
				m_Points[m_NumPoints] = Vector(x, y, z) + position;
				m_NumPoints++;
			}
			else if (tokens[0] == "f")
			{
				int x = tokens[1].ToInt() - 1;
				int y = tokens[2].ToInt() - 1;
				int z = tokens[3].ToInt() - 1;
				
				m_Indices[m_NumIndices + 0] = x;
				m_Indices[m_NumIndices + 1] = y;
				m_Indices[m_NumIndices + 2] = z;
				m_NumIndices += 3;
			}
		}
		
		return true;
	}

	bool CreateFromBoundingBox()
	{
		if (!m_Position)
		{
			return false;
		}

		m_NumPoints = 8;
		m_NumIndices = 36;

		vector box[2];
		vector transform[4];

		m_Position.GetTransform(transform);

		vector position = transform[3];

		box[0][0] = position[0] - m_Size[0];
		box[0][1] = position[1] - m_Size[1];
		box[0][2] = position[2] - m_Size[2];

		box[1][0] = position[0] + m_Size[0];
		box[1][1] = position[1] + m_Size[1];
		box[1][2] = position[2] + m_Size[2];

		m_Points[0] = Vector(box[0][0], box[0][1], box[0][2]);
		m_Points[1] = Vector(box[1][0], box[0][1], box[0][2]);
		m_Points[2] = Vector(box[1][0], box[1][1], box[0][2]);
		m_Points[3] = Vector(box[0][0], box[1][1], box[0][2]);
		m_Points[4] = Vector(box[0][0], box[0][1], box[1][2]);
		m_Points[5] = Vector(box[1][0], box[0][1], box[1][2]);
		m_Points[6] = Vector(box[1][0], box[1][1], box[1][2]);
		m_Points[7] = Vector(box[0][0], box[1][1], box[1][2]);
		
		m_Indices[0] = 0;
		m_Indices[1] = 1;
		m_Indices[2] = 3;

		m_Indices[3] = 3;
		m_Indices[4] = 1;
		m_Indices[5] = 2;

		m_Indices[6] = 1;
		m_Indices[7] = 5;
		m_Indices[8] = 2;

		m_Indices[9] = 2;
		m_Indices[10] = 5;
		m_Indices[11] = 6;

		m_Indices[12] = 5;
		m_Indices[13] = 4;
		m_Indices[14] = 6;

		m_Indices[15] = 6;
		m_Indices[16] = 4;
		m_Indices[17] = 7;

		m_Indices[18] = 4;
		m_Indices[19] = 0;
		m_Indices[20] = 7;

		m_Indices[21] = 7;
		m_Indices[22] = 0;
		m_Indices[23] = 3;

		m_Indices[24] = 3;
		m_Indices[25] = 2;
		m_Indices[26] = 7;

		m_Indices[27] = 7;
		m_Indices[28] = 2;
		m_Indices[29] = 6;

		m_Indices[30] = 4;
		m_Indices[31] = 5;
		m_Indices[32] = 0;

		m_Indices[33] = 0;
		m_Indices[34] = 5;
		m_Indices[35] = 1;

		return true;
	}

	void RenderDebug(R3D_VehicleSimulationComponent simulation)
	{
		int i;
		
		vector triangles[MAX_INDICES];
		for (i = 0; i < m_NumIndices; i += 3)
		{
			triangles[i + 0] = m_Points[m_Indices[i + 0]];
			triangles[i + 1] = m_Points[m_Indices[i + 1]];
			triangles[i + 2] = m_Points[m_Indices[i + 2]];
		}

		for (i = 0; i < m_NumPoints; i++)
		{
		//	simulation.Debug_Add(R3D_DebugShape.CreateSphere(0xFFFFFFFF, ShapeFlags.NOOUTLINE | ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, m_Points[i], 0.05));
		}
		
		for (i = 0; i < m_NumIndices; i += 3)
		{
		//	simulation.Debug_Add(R3D_DebugShape.CreateLine(0xFFFF0000, ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, m_Points[m_Indices[i + 0]], m_Points[m_Indices[i + 1]]));
		//	simulation.Debug_Add(R3D_DebugShape.CreateLine(0xFF0000FF, ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, m_Points[m_Indices[i + 1]], m_Points[m_Indices[i + 2]]));
		//	simulation.Debug_Add(R3D_DebugShape.CreateLine(0xFF00FF00, ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, m_Points[m_Indices[i + 2]], m_Points[m_Indices[i + 0]]));
		}
		
		for (i = 0; i < m_NumIndices; i += 3)
		{
			int i1 = m_Indices[i + 0];
			int i2 = m_Indices[i + 1];
			int i3 = m_Indices[i + 2];

			vector v1 = m_Points[i1];
			vector v2 = m_Points[i2];
			vector v3 = m_Points[i3];
			
			vector normal = ((v2 - v1) * (v3 - v1)).Normalized();
			vector position = (v1 + v2 + v3) * (1.0 / 3.0);

			simulation.Debug_Add(R3D_DebugShape.CreateSphere(0xFFFF0000, ShapeFlags.NOOUTLINE | ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, position, 0.025));
			simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFFFF0000, ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, position, normal));
		}
		
		simulation.Debug_Add(R3D_DebugShape.CreateTris(0x10EEEEEE, ShapeFlags.NOOUTLINE | ShapeFlags.DOUBLESIDE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, triangles, m_NumIndices / 3));	
	}
};
