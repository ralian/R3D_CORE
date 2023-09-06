/*
 * The 'r' prefix denotes runtime variables. This is because the component 
 * system is intended to be split into data classes for faster runtime 
 * processing after all the work is done. Refer to ticket 
 */

class R3D_BuoyancyComponent : R3D_VehiclePartBaseComponent
{
	//! REMEMBER: Keep same with 'R3D_BuoyancyComponentClass'
	static const int MAX_POINTS = 128;
	static const int MAX_INDICES = 1024;
	static const int MAX_PARTICLES = 2048;

	[Attribute(defvalue: "0.000001")]
	float m_LinearDamp;

	[Attribute(defvalue: "0.000001")]
	float m_LinearDampSquared;

	[Attribute(defvalue: "100.0")]
	float m_AngularDamp;

	[Attribute(defvalue: "100.0")]
	float m_AngularDampSquared;
	
	[Attribute(defvalue: "100.0")]
	float m_CPDrag1;
	
	[Attribute(defvalue: "100.0")]
	float m_CPDrag2;
	
	[Attribute(defvalue: "0.4")]
	float m_FPDrag;

	[Attribute(defvalue: "100.0")]
	float m_CSDrag1;
	
	[Attribute(defvalue: "100.0")]
	float m_CSDrag2;
	
	[Attribute(defvalue: "0.4")]
	float m_FSDrag;

	[Attribute(defvalue: "1.0")]
	float m_VRDrag;

	[Attribute(defvalue: "0.2")]
	float m_SlamCoeff;

	float m_rDepths[MAX_POINTS];
	vector m_rPoints[MAX_POINTS];
	
	int m_rNumSubmergedTriangles;
	vector m_rSubmergedTrianglePoints[MAX_INDICES];
	float m_rTriangleAreas[MAX_INDICES];

	float m_rWaterLineSize;
	float m_rReynoldsNumber;
	float m_rViscousDragCoefficient;

	float m_rVolume;
	float m_rArea;

	vector m_rBuoyantPosition;

	void Tetrahedron(inout int triangle, float dt, Physics physics)
	{
		vector v1 = m_rSubmergedTrianglePoints[triangle++];
		vector v2 = m_rSubmergedTrianglePoints[triangle++];
		vector v3 = m_rSubmergedTrianglePoints[triangle++];

		vector normal = (v2 - v1) * (v3 - v1);
		float volume = vector.Dot(v1, v2 * v3) / 6.0;
				
		float area = normal.NormalizeSize() * 0.5;
		
		m_rVolume += volume;
		m_rArea += area;
		
		vector position = (v1 + v2 + v3) * (1.0 / 3.0);
		m_rBuoyantPosition += volume * position;

		vector velocity = m_Simulation.GetModelVelocityAt(position);
		float velocityMagnitude = velocity.NormalizeSize();

		float cosTheta = vector.Dot(normal, velocity);
		
		vector velocityNormal = normal * cosTheta;
		vector velocityTangent = (velocity - velocityNormal);
		velocityTangent.Normalize();
		
		vector tangentFlow = -velocityTangent * velocityMagnitude;
					
		float v = velocityMagnitude / m_VRDrag;
		float pressureDragMagnitude = 0.0;
		if (cosTheta >= 0.0)
		{
			pressureDragMagnitude = -((m_CPDrag1 * v) + (m_CPDrag2 * v * v)) * area * Math.Pow(cosTheta, m_FPDrag);
		}
		else
		{
			pressureDragMagnitude =  ((m_CSDrag1 * v) + (m_CSDrag2 * v * v)) * area * Math.Pow(-cosTheta, m_FSDrag);
		}
		
		vector viscousDrag = tangentFlow * 0.5 * R3D_Physics.WaterDensity * area * m_rViscousDragCoefficient * velocityMagnitude;
		vector pressureDrag = normal * pressureDragMagnitude;
		
		vector force = viscousDrag + pressureDrag;
	
		//m_Simulation.Debug_Add(R3D_DebugShape.CreateTriangle(0x44FF4433, ShapeFlags.NOOUTLINE | ShapeFlags.DOUBLESIDE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, v1, v2, v3));
		//m_Simulation.Debug_Add(R3D_DebugShape.CreateSphere(0xFF000000, ShapeFlags.NOOUTLINE | ShapeFlags.NOZBUFFER, position, 0.025));
		//m_Simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFF00FF00, ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, position, viscousDrag));
		//m_Simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFFFFFF00, ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, position, pressureDrag));
		//m_Simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFFFF0000, ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, position, force));
		
		m_Simulation.VMultiply3(force, force);
		m_Simulation.VMultiply4(position, position);

		physics.ApplyImpulseAt(position, force * dt);
	}

	float ClipTriangle(vector normal, float dt, vector v1, vector v2, vector v3, float d1, float d2, float d3)
	{
		vector vc1 = v1 + ((d1 / (d1 - d2)) * (v2 - v1));
		vector vc2;

		float area;

		if (d1 < 0)
		{
			if (d3 < 0)
			{
				vc2 = v2 + ((d2 / (d2 - d3)) * (v3 - v2));

				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc1;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc2;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v1;

				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc2;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v3;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v1;

				area += ((vc2 - vc1) * (v1 - vc1)).Length() * 0.5;
				area += ((v3 - vc2) * (v1 - vc2)).Length() * 0.5;
			}
			else
			{
				vc2 = v1 + ((d1 / (d1 - d3)) * (v3 - v1));

				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc1;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc2;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v1;

				area += ((vc2 - vc1) * (v1 - vc1)).Length() * 0.5;
			}
		}
		else
		{
			if (d3 < 0)
			{
				vc2 = v1 + ((d1 / (d1 - d3)) * (v3 - v1));

				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc1;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v2;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v3;

				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc1;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v3;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc2;

				area += ((v2 - vc1) * (v3 - vc1)).Length() * 0.5;
				area += ((v3 - vc1) * (vc2 - vc1)).Length() * 0.5;
			}
			else
			{
				vc2 = v2 + ((d2 / (d2 - d3)) * (v3 - v2));

				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc1;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v2;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = vc2;

				area += ((v2 - vc1) * (vc2 - vc1)).Length() * 0.5;
			}
		}

		float length = Math.AbsFloat((vc1 - vc2).Length());
		m_rWaterLineSize += length;
		
		//m_Simulation.Debug_Add(R3D_DebugShape.CreateLine(0xFFFFFFFF, ShapeFlags.NOZBUFFER, vc1, vc2));

		return area;
	}

	override void OnSimulate(float dt, Physics physics)
	{
		R3D_BuoyancyComponentClass src = R3D_BuoyancyComponentClass.Cast(GetComponentData(GetOwner()));
		if (!src)
		{
			return;
		}

		BaseWorld world = GetOwner().GetWorld();
		
		int i;
		vector buoyantPosition;
		int numSubmerged;
		int submergedTriangle;

		m_rNumSubmergedTriangles = 0;
		m_rWaterLineSize = 0;
		m_rVolume = 0;
		m_rArea = 0;
		m_rBuoyantPosition = vector.Zero;

		for (i = 0; i < src.m_NumPoints; i++)
		{
			vector temp;
			
			
			m_Simulation.VMultiply4(src.m_Points[i],temp);
			m_rPoints[i] = temp;
		
			/*
			vector waterSurfacePoint;
			EWaterSurfaceType outType;
			vector transformWS[4];
			vector obbExtents;
			
			ChimeraWorldUtils.TryGetWaterSurface(world, m_rPoints[i], waterSurfacePoint, outType, transformWS, obbExtents);

			*/

			vector waterSurfacePoint = Vector(m_rPoints[i][0], 0, m_rPoints[i][2]);
			waterSurfacePoint[1] = world.GetOceanBaseHeight() + world.GetOceanHeight(m_rPoints[i][0], m_rPoints[i][2]);

			m_rDepths[i] = m_rPoints[i][1] - waterSurfacePoint[1];
			
			//vector waterSurfacePointMS = m_Simulation.VInvMultiply4(waterSurfacePoint);
			//m_Simulation.Debug_Add(R3D_DebugText.Create("Depth: " + m_rDepths[i], waterSurfacePointMS + Vector(0, 0.3, 0)));
			//m_Simulation.Debug_Add(R3D_DebugShape.CreateSphere(0xFF00FF00, ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, waterSurfacePointMS, 0.025));	

			if (m_rDepths[i] <= 0)
			{
				++numSubmerged;
			}
		}

		if (numSubmerged == 0)
		{
			return;
		}

		for (i = 0; i < src.m_NumIndices; i += 3)
		{
			int i1 = src.m_Indices[i + 0];
			int i2 = src.m_Indices[i + 1];
			int i3 = src.m_Indices[i + 2];

			vector v1 = src.m_Points[i1];
			float d1 = m_rDepths[i1];
			vector v2 = src.m_Points[i2];
			float d2 = m_rDepths[i2];
			vector v3 = src.m_Points[i3];
			float d3 = m_rDepths[i3];

			int triangleIndex = i / 3;
			vector triangleNormal = ((v2 - v1) * (v3 - v1));
			float triangleMaxArea = triangleNormal.NormalizeSize() * 0.5;
			float triangleArea = triangleMaxArea;

			if (d1 * d2 < 0)
			{
				triangleArea = ClipTriangle(triangleNormal, dt, v1, v2, v3, d1, d2, d3);
			}
			else if (d1 * d3 < 0)
			{
				triangleArea = ClipTriangle(triangleNormal, dt, v3, v1, v2, d3, d1, d2);
			}
			else if (d2 * d3 < 0)
			{
				triangleArea = ClipTriangle(triangleNormal, dt, v2, v3, v1, d2, d3, d1);
			}
			else if (d1 < 0 || d2 < 0 || d3 < 0)
			{
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v1;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v2;
				m_rSubmergedTrianglePoints[m_rNumSubmergedTriangles++] = v3;
			}
			else
			{
				// The triangle isn't in the water
				m_rTriangleAreas[triangleIndex] = 0;
				continue;
			}
			
			float trianglePreviousArea = m_rTriangleAreas[triangleIndex];
			m_rTriangleAreas[triangleIndex] = triangleArea;
			
			vector trianglePosition = (v1 + v2 + v3) * (1.0 / 3.0);
			vector triangleVelocity = m_Simulation.GetModelVelocityAt(trianglePosition);
			float triangleVelocityMagnitude = triangleVelocity.NormalizeSize();
				
			float triangleSurfaceProjection = Math.Max(vector.Dot(triangleNormal, triangleVelocity), 0.0);
			
			float triangleAreaVelocity = Math.Max((triangleArea - trianglePreviousArea) * triangleSurfaceProjection, 0.0) + (triangleVelocityMagnitude * triangleSurfaceProjection * 0.9);
			
			triangleAreaVelocity *= m_SlamCoeff;
			
			float triangleAreaVelocitySq = triangleAreaVelocity * triangleAreaVelocity * triangleAreaVelocity;
			
			triangleAreaVelocity = Math.Clamp(triangleAreaVelocitySq, 0.0, triangleAreaVelocity) / m_SlamCoeff;
			
			float slamForceMagnitude = R3D_Physics.WaterDensity * triangleAreaVelocity * src.m_Area * triangleSurfaceProjection;
			
			vector slamForce = -triangleNormal * slamForceMagnitude * dt;

			//m_Simulation.Debug_Add(R3D_DebugShape.CreateSphere(0x88FF0000, ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, trianglePosition, 0.025));
			//m_Simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFF00FF00, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, trianglePosition, triangleNormal * triangleSurfaceProjection * triangleAreaAcceleration));
			//m_Simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFFFF0000, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, trianglePosition, slamForce));

			m_Simulation.VMultiply3(slamForce, slamForce);
			m_Simulation.VMultiply4(trianglePosition, trianglePosition);
			
			physics.ApplyImpulseAt(trianglePosition, slamForce);
		}

		m_rWaterLineSize *= 0.5;

		m_rReynoldsNumber = m_Simulation.m_LinearVelocity.Length() * m_rWaterLineSize / R3D_Physics.WaterKinematicViscosity;
		
		m_rViscousDragCoefficient = R3D_Math.Log10(Math.Max(m_rReynoldsNumber, 1.0e3)) - 2.0;
		m_rViscousDragCoefficient *= m_rViscousDragCoefficient;
		m_rViscousDragCoefficient = 0.075 / m_rViscousDragCoefficient;
		
		//AddDebugText("Waterline", m_rWaterLineSize);
		//AddDebugText("Reynolds Number", m_rReynoldsNumber);
		//AddDebugText("Viscous Drag", m_rViscousDragCoefficient);

		while (submergedTriangle < m_rNumSubmergedTriangles)
		{
			Tetrahedron(submergedTriangle, dt, physics);
		}
		
		//AddDebugText("Mass", m_Simulation.m_Mass);
		//AddDebugText("Submerged Points", numSubmerged);
		//AddDebugText("Submerged Triangles", m_rNumSubmergedTriangles);
		//AddDebugText("Optimal Mass", optimalMass);
		//AddDebugText("Volume Submerged", m_rVolume);
		//AddDebugText("Volume Total", src.m_Volume);
		//AddDebugText("Area Submerged", m_rArea);
		//AddDebugText("Area Total", src.m_Area);

		if (m_rVolume > 0.00001)
		{
			m_rBuoyantPosition = m_rBuoyantPosition * (1.0 / m_rVolume);
		
			vector velocity = m_Simulation.GetModelVelocityAt(m_rBuoyantPosition);
			vector velocityWS; 
			m_Simulation.VMultiply3(-velocity,velocityWS);
			vector buoyantPositionWS;
			m_Simulation.VMultiply4(m_rBuoyantPosition,buoyantPositionWS);

			vector buoyancyForce = -Physics.VGravity * R3D_Physics.WaterDensity * m_rVolume;

			float areaRatio = m_rArea / src.m_Area;

			vector linear = m_Simulation.m_LinearVelocity;
			float linSpeed = linear.NormalizeSize();
			float cL = -areaRatio * (m_LinearDamp + m_LinearDampSquared * linSpeed);
			vector stabilizeForce = linear * cL;

			vector angular = m_Simulation.m_AngularVelocity;
			float angSpeed = angular.NormalizeSize();
			float cR = -areaRatio * (m_AngularDamp + m_AngularDampSquared * angSpeed);    
			vector stabilizeTorque = angular * cR;

			//FIXME: Might be the other way around?
			vector stabilizePosition = stabilizeTorque * stabilizeForce;

			physics.ApplyImpulseAt(stabilizePosition, stabilizeForce * dt);
			physics.ApplyImpulseAt(buoyantPositionWS, buoyancyForce * dt);
		}
	}

	void OnDebugUpdate(float dt)
	{
		R3D_BuoyancyComponentClass src = R3D_BuoyancyComponentClass.Cast(GetComponentData(GetOwner()));
		if (!src)
		{
			return;
		}
		
		src.RenderDebug(m_Simulation);
	}

	vector GetDebugPosition()
	{
		return "0 5 0";
	}
};
