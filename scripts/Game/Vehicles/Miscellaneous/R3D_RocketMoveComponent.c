[BaseContainerProps()]
class ADM_MissileAerodynamicSurface
{
	
	[Attribute(desc: "Aerodynamic Center and Orientation of Surface. +Y should be normal, +Z should be chord")]
	protected ref PointInfo m_vCoordinateSystem;
	
	[Attribute(params: "0 inf")]
	protected float m_fSurfaceArea; 
	
	protected float m_fSweepAngle;
	protected float m_fDihedralAngle;
	protected float m_fTwistAngle;
	
	void Init(IEntity owner)
	{
		if (m_vCoordinateSystem)
		{
			m_vCoordinateSystem.Init(owner);
			vector mat[4];
			m_vCoordinateSystem.GetTransform(mat);
			
			vector Q[3];
			Q[0] = mat[0];
			Q[1] = mat[1];
			Q[2] = mat[2];
			vector angles = Math3D.MatrixToAngles(Q);
			
			m_fSweepAngle = angles[0];
			m_fDihedralAngle = angles[1];
			m_fTwistAngle = angles[2];
		}
	}
	
	
	float GetSweepAngle()
	{
		return m_fSweepAngle;
	}
	
	float GetDihedralAngle()
	{
		return m_fDihedralAngle;
	}
	
	float GetTwistAngle()
	{
		return m_fSweepAngle;
	}
	
}

class R3D_RocketMoveComponentClass: ADM_RigidbodyComponentClass {}
class R3D_RocketMoveComponent: ADM_RigidbodyComponent
{
	// Vehicle Specifications
	[Attribute(category: "Mass Parameters", defvalue: "10", uiwidget: UIWidgets.EditBox)]
	protected float m_fStructuralMass; // [kg]
	
	[Attribute(category: "Mass Parameters", defvalue: "70", uiwidget: UIWidgets.EditBox)]
	protected float m_fPropellantMass; // [kg]
	
	[Attribute(category: "Mass Parameters", defvalue: "20", uiwidget: UIWidgets.EditBox)]
	protected float m_fPayloadMass; // [kg]
	
	[Attribute(category: "Thrust Vectoring", defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Degrees")]
	protected float m_fMaxConeAngle;
	
	[Attribute(category: "Engine Parameters", UIWidgets.ResourcePickerThumbnail, "Prefab of particle", params: "ptc")]
	protected ResourceName m_ExhaustParticle;
	
	// Engine Parameters
	[Attribute(category: "Engine Parameters", defvalue: "250", uiwidget: UIWidgets.EditBox, desc: "Specific impulse")]
	protected float m_fIsp; // [s]
	
	[Attribute(category: "Engine Parameters", defvalue: "100", uiwidget: UIWidgets.EditBox)]
	protected float m_fBurnTime; // [s]
	
	[Attribute(category: "Engine Parameters", defvalue: "250", uiwidget: UIWidgets.Object, desc: "Exhaust particles & thrust location")]
	protected ref PointInfo m_vExhaustLocation;
	
	// Aerodynamic Surfaces
	[Attribute(category: "Aerodynamics")]
	protected ref array<ref ADM_MissileAerodynamicSurface> m_AeroSurfaces;
	
	[Attribute("0 0 0", UIWidgets.EditBox, params: "inf inf 0 purpose=coords space=entity", category: "Aerodynamics")]
	protected vector m_vAeroCenter;
	
	[Attribute(params: "0 1", defvalue: "1", category: "Aerodynamics")]
	protected float m_fWindStrength;
	
	[Attribute(params: "0 inf", category: "Aerodynamics")]
	protected float m_fFrontalDragArea;
	
	[Attribute(params: "0 inf", category: "Aerodynamics")]
	protected float m_fFrontalDragCoefficient;
	
	[Attribute(params: "0 inf", category: "Aerodynamics")]
	protected float m_fSideDragArea;
	
	[Attribute(params: "0 inf", category: "Aerodynamics")]
	protected float m_fSideDragCoefficient;
	
	protected WorldTimestamp m_fLaunchTime;	
	protected float m_fMassFlowRate;
	protected float m_fDryMass;
	protected vector m_vExhaustPosition;
	protected vector m_vWorldThrustDirection;
	protected ChimeraWorld m_World;
	protected TimeAndWeatherManagerEntity m_TimeManager = null;
	
	protected float m_fThrustAngleX; // Thrust angle about body x axis [radians]
	protected float m_fThrustAngleY; // Thrust angle about body y axis [radians]
	
	float GetThrustAngleX()
	{
		return m_fThrustAngleX;
	}
	
	float GetThrustAngleY()
	{
		return m_fThrustAngleY;
	}
	
	// Set in degrees
	void SetThrustAngleX(float angle)
	{
		if (angle < m_fMaxConeAngle*-1) angle = m_fMaxConeAngle*-1;
		if (angle > m_fMaxConeAngle) angle = m_fMaxConeAngle;
		
		m_fThrustAngleX = angle;
	}
	
	// Set in degrees
	void SetThrustAngleY(float angle)
	{
		if (angle < m_fMaxConeAngle*-1) angle = m_fMaxConeAngle*-1;
		if (angle > m_fMaxConeAngle) angle = m_fMaxConeAngle;
		
		m_fThrustAngleY = angle;
	}
	
	float GetAltitude(IEntity owner)
	{
		return COM[1];
	}
	
	float GetTimeUntilBurnout()
	{
		float timeSinceLaunch = m_World.GetTimestamp().DiffMilliseconds(m_fLaunchTime)/1000; // [s]
		return m_fBurnTime - timeSinceLaunch;
	}
	
	float GetMachNumber(IEntity owner, float speed)
	{
		float speedOfSound = ADM_InternationalStandardAtmosphere.GetValue(GetAltitude(owner), ADM_ISAProperties.SpeedOfSound);
		float mach = speed / speedOfSound;
		
		return mach;
	}
	
	void Launch()
	{
		if (!m_Physics)
			return;
		
		m_fLaunchTime = m_World.GetTimestamp();
		m_Physics.SetActive(true);
	}
	
	void Setup(IEntity owner)
	{
		m_fMassFlowRate = m_fPropellantMass / m_fBurnTime;
		m_fDryMass = m_fStructuralMass + m_fPayloadMass;
		m_fMaxConeAngle = m_fMaxConeAngle * Math.DEG2RAD;
		
		if (m_vExhaustLocation) {
			m_vExhaustLocation.Init(owner);
			vector exhaustTransform[4];
			m_vExhaustLocation.GetLocalTransform(exhaustTransform);
			m_vExhaustPosition = exhaustTransform[3];
		}
			
		if (!m_Physics)
			return;
		
		m_Physics.SetMass(m_fDryMass + m_fPropellantMass);
	}
	
	vector GetWindVector()
	{		
		if (!m_TimeManager) 
			return vector.Zero;
		
		vector windAngles = vector.Zero;
		float speed = m_TimeManager.GetWindSpeed();
		windAngles[0] = m_TimeManager.GetWindDirection();
		
		vector mat[3];
		Math3D.AnglesToMatrix(windAngles, mat);
		
		return speed * mat[2] * m_fWindStrength;
	}
	
	override event void OnPostInit(IEntity owner)
	{
		Setup(owner);
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME);
		
		foreach(ADM_MissileAerodynamicSurface surf : m_AeroSurfaces)
		{
			surf.Init(owner);
		}
		
		m_World = owner.GetWorld();
		if (m_World) 
		{
			m_TimeManager = m_World.GetTimeAndWeatherManager();
		}
		
		Launch();
	}
	
	float bodyDynamicPressure, bodyAttackAngle, bodySideSlipAngle, bodyDrag, bodySideDrag;
	vector wind;
	override void UpdateForcesAndMoments(IEntity owner, float curTime = System.GetTickCount())
	{
		super.UpdateForcesAndMoments(owner);
		
		// gravity
		//forces += m_fMass * Physics.VGravity;
		
		// thrust
		float timeUntilBurnout = GetTimeUntilBurnout();
		if (timeUntilBurnout >= 0) {
			float mass = m_fDryMass + m_fPropellantMass*timeUntilBurnout/m_fBurnTime;
			float thrust = m_fIsp * m_fMassFlowRate * 9.81; // Isp = T/mdot/g_e,
			//forces += Q[2]*thrust;
			//moments += m_vExhaustPosition * "0 0 1" * thrust;
			m_fMass = mass;			
		}
		
		// Aerodynamics		
		wind = GetWindVector();
		wind = "100 50 0";
		float altitude = GetAltitude(owner);
		float density = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Density); // [kg/m^3]
		
		// Fuselage
		vector bodyFlowVelocity = v + wind;
		vector bodyFlowVelocityLocal = VectorToLocal(bodyFlowVelocity);
		bodyDynamicPressure = 0.5 * density * bodyFlowVelocity.LengthSq(); 
		bodyAttackAngle = Math.Atan2(-bodyFlowVelocityLocal[1], bodyFlowVelocityLocal[2]);	
		bodySideSlipAngle = Math.Atan2(-bodyFlowVelocityLocal[0], bodyFlowVelocityLocal[2]);	
		
		// Aerodynamic forces calculated in Wind Axis frame
		// + X = opposite direction of airflow (opposite direction of drag)
		// + Y = cross(z,y)
		// + Z = lift direction, vertical, pointing down relative to aircraft
		// https://society-of-flight-test-engineers.github.io/handbook-2013/axis-systems-and-transformations-raw.html
		// reforger:
		// +X = right (wind +Y)
		// +Y = up (wind +Z)
		// +Z = forward (wind +X)
		
		//bodyDrag = bodyDynamicPressure*m_fFrontalDragArea * m_fFrontalDragCoefficient;
		//bodySideDrag = bodyDynamicPressure*m_fSideDragArea * (m_fSideDragCoefficient*bodySideSlipAngle);
		
		// Aerodynamic forces are applied in Body Axis frame
		//forces += dragDir*bodyDrag;
		//forces += Q[0]*bodySideDrag;
		//moments += m_vAeroCenter*(Q[0]*bodySideDrag);
		
		// Aerodynamic Surfaces
		foreach (ADM_MissileAerodynamicSurface surf : m_AeroSurfaces)
		{
			//vector sectionFlowVelocity = m_Physics.GetVelocityAt(aerocenter) + wind;
			//vector sectionFlowVelocityLocal = m_Owner.VectorToLocal(sectionFlowVelocity);
			//float sectionDynamicPressure = 0.5 * density * sectionFlowVelocity.LengthSq(); // [Pa]
			
			//vector dragDir = sectionFlowVelocity.Normalized();
			//vector liftDir = dragDir * vSpan;
			
			//float angleOfAttack = Math.Atan2(-sectionFlowVelocityLocal[1], sectionFlowVelocityLocal[2])*Math.RAD2DEG;
		}
	} 
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		//float timeUntilBurnout = GetTimeUntilBurnout();
		//if (timeUntilBurnout <= 0 && m_pParticle.GetIsPlaying())
		//	m_pParticle.GetParticles().SetParam(-1, EmitterParam.BIRTH_RATE, 0);
		
		DbgUI.Begin("Rocket Move Component", 0, 0);
		
		DbgUI.Text(string.Format("AoA: %1 deg", Math.Round(bodyAttackAngle*100*Math.RAD2DEG)/100));
		DbgUI.Text(string.Format("Side: %1 deg", Math.Round(bodySideSlipAngle*100*Math.RAD2DEG)/100));
		
		DbgUI.Text(string.Format("m_fMass: %1 kg", Math.Round(m_fMass*100)/100));
		DbgUI.Text(string.Format("Y: %1 m", Math.Round(COM[1]*100)/100));
		DbgUI.Text(string.Format("speed: %1 m/s", Math.Round(v.Length()*100)/100));
		
		DbgUI.End();
		
		// Body Axes
		//Shape.CreateArrow(COM, COM + Q[0] * 2, 0.1, COLOR_RED, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		//Shape.CreateArrow(COM, COM + Q[1] * 2, 0.1, COLOR_GREEN, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		//Shape.CreateArrow(COM, COM + Q[2] * 2, 0.1, COLOR_BLUE, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		//Shape.CreateArrow(owner.CoordToParent(m_vExhaustPosition), owner.CoordToParent(m_vExhaustPosition) + m_vWorldThrustDirection*-1, 0.1, COLOR_RED, ShapeFlags.ONCE); 

		Shape.CreateArrow(COM, COM + wind * 2, 0.1, COLOR_YELLOW, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		//Shape.CreateArrow(COM, COM + windAx[0] * 2, 0.1, COLOR_RED, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		//Shape.CreateArrow(COM, COM + windAx[1] * 2, 0.1, COLOR_GREEN, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		//Shape.CreateArrow(COM, COM + windAx[2] * 2, 0.1, COLOR_BLUE, ShapeFlags.NOZBUFFER | ShapeFlags.ONCE);
		
	}

	
	/*
	
	vector CalculateTrajectoryCollision(IEntity object)
	{
		// Assume basic projectile motion, no drag. Calculate collision point on terrain
		vector initialPosition = object.GetOrigin();
		vector initialVelocity = vector.Zero;
		if (object.GetPhysics()) initialVelocity = object.GetPhysics().GetVelocity();
		
		vector collision = initialPosition; // default to current position if no solution found within the set timeframe
		for (float t = 0; t < 100; t += 0.01)
		{
			vector position = initialPosition + initialVelocity * t + 1/2 * Physics.VGravity * t * t;
			float terrainHeight = SCR_TerrainHelper.GetTerrainY(position);
			if (position[1] <= terrainHeight) {
				collision = position;	
				break;							
			}
		}
		
		return collision;
	}
	
	vector worldThrustPosition;
	vector worldThrustDirection;
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_Physics || m_fLaunchTime == -1)
			return;
		
		// Update mass & thrust
		float timeUntilBurnout = GetTimeUntilBurnout();
		if (timeUntilBurnout >= 0) {
			float mass = m_fDryMass + m_fPropellantMass*timeUntilBurnout/m_fBurnTime;
			float thrust = m_fIsp * m_fMassFlowRate * 9.81; // Isp = T/mdot/g_e
			//Print(thrust);
		
			m_Physics.ApplyForceAt(worldThrustPosition, thrust * worldThrustDirection);
			m_Physics.SetMass(mass);
		}
		
		//if (timeUntilBurnout <= 0 && m_pParticle.GetIsPlaying())
		//	m_pParticle.GetParticles().SetParam(-1, EmitterParam.BIRTH_RATE, 0);
		
		previousVelocity = m_Physics.GetVelocity();
		
		// Use Z as it is
		vector mat[4];
		owner.GetTransform(mat);
		
		vector x = mat[0];
		x[1] = 0; // remove y component
		x.Normalize(); // renormalize
		
		// calculate x as cross product
		vector z = mat[2];
		vector y = z*x;
		
		vector newMat[4];
		newMat[0] = x;
		newMat[1] = y;
		newMat[2] = z;
		newMat[3] = mat[3];
		owner.SetTransform(mat);
	}*/
	
	/*bool m_ShowDbgUI = true;
	vector previousVelocity = vector.Zero;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
	
		worldThrustPosition = m_Owner.CoordToParent(m_ExhaustPosition);
			
		vector localThrustDirection = "0 0 1";
		
		// ------- Rotation
		float sinX = Math.Sin(m_fThrustAngleX);
		float cosX = Math.Cos(m_fThrustAngleX);
		
		float sinZ = Math.Sin(m_fThrustAngleY);
		float cosZ = Math.Cos(m_fThrustAngleY);
		
		//rotation matrix start X-axis
		vector rotMatX[3];
		rotMatX[0][0] = 1;
		rotMatX[0][1] = 0;
		rotMatX[0][2] = 0;
		
		rotMatX[1][0] = 0;
		rotMatX[1][1] = cosX;
		rotMatX[1][2] = -sinX;
		
		rotMatX[2][0] = 0;
		rotMatX[2][1] = sinX;
		rotMatX[2][2] = cosX;	
		//rotation matrix end X-axis
		
		//rotation matrix start Z-axis
		vector rotMatZ[3];
		rotMatZ[0][0] = cosZ;
		rotMatZ[0][1] = -sinZ;
		rotMatZ[0][2] = 0;
		
		rotMatZ[1][0] = sinZ;
		rotMatZ[1][1] = cosZ;
		rotMatZ[1][2] = 0;
		
		rotMatZ[2][0] = 0;
		rotMatZ[2][1] = 0;
		rotMatZ[2][2] = 1;	
		//rotation matrix end Z-axis
		
		vector rotatedThrustDirectionX;
		vector rotatedThrustDirectionXZ;
		Math3D.MatrixMultiply3(rotMatX, localThrustDirection, rotatedThrustDirectionX);
		Math3D.MatrixMultiply3(rotatedThrustDirectionX, rotMatZ, rotatedThrustDirectionXZ);
		// ------- Rotation
					
		worldThrustDirection = m_Owner.VectorToParent(rotatedThrustDirectionXZ);
		worldThrustDirection.Normalize();
	
		/*float amplitude = m_fMaxConeAngle;
		float angleX = amplitude * Math.Sin(System.GetTickCount() / 1000 * 2 * Math.PI);
		float angleY = amplitude * Math.Cos(System.GetTickCount() / 1000 * 2 * Math.PI);
		SetThrustAngleX(angleX);
		SetThrustAngleY(angleY);
		
		Print(angleX);
		Print(angleY);
	}*/
}