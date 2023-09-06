class R3D_RocketMoveComponentClass: ScriptComponentClass {}

class R3D_RocketMoveComponent: ScriptComponent
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

	// Aerodynamics
	[Attribute(category: "Aerodynamics", defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Reference drag surface area")]
	protected float m_fDragSurfaceArea; // [m^2]
	
	[Attribute(category: "Aerodynamics", defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Reference lift surface area")]
	protected float m_fLiftSurfaceArea; // [m^2]
	
	[Attribute(category: "Aerodynamics", defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Skin friction coefficient of vehicle")]
	protected float m_fCF0; // [unitless]
	
	[Attribute(category: "Aerodynamics", defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Drag coefficient due to bluntness")]
	protected float m_fCD0; // [unitless]
	
	[Attribute(category: "Aerodynamics")]
	protected vector m_aerodynamicCenter;
	
	protected IEntity m_Owner;
	protected Physics m_Physics;
	
	protected float m_fLaunchTime = -1;	
	protected float m_fMassFlowRate;
	protected float m_fDryMass;
	protected vector m_ExhaustPosition;
	
	protected float m_fThrustAngleX; // Thrust angle about body x axis [radians]
	protected float m_fThrustAngleY; // Thrust angle about body y axis [radians]
	
	protected SCR_ParticleEmitter m_pParticle;
	
	override void EOnInit(IEntity owner)
	{
		m_Owner = owner;
		m_Physics = m_Owner.GetPhysics();
		
		Setup();
	}
	
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.SIMULATE);
	}
	
	static vector CrossProduct(vector v1, vector v2)
	{
		// Right hand coordinate system
		// a_2 * b_3 - a_3 * b_2
		// -(a_1 * b_3 - a_3 * b_1)
		// a_1 * b_2 - a_2 * b_1
		vector cross;
		cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
		cross[1] = v1[0] * v2[2] - v1[2] * v2[0];
		cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
		
		return cross;
	}
	
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
	
	float GetAltitude()
	{
		if (!m_Owner)
			return -1;
		
		return m_Owner.GetOrigin()[1];
	}
	
	float GetTimeUntilBurnout()
	{
		float timeSinceLaunch = (System.GetTickCount() - m_fLaunchTime)/1000; // [s]
		return m_fBurnTime - timeSinceLaunch;
	}
	
	float GetMachNumber()
	{
		float speedOfSound = ADM_InternationalStandardAtmosphere.GetValue(GetAltitude(), ADM_ISAProperties.SpeedOfSound);
		vector vel = m_Physics.GetVelocity();
		float mach = vel.Length() / speedOfSound;
		
		return mach;
	}
	
	void Launch()
	{
		if (!m_Physics)
			return;
		
		m_fLaunchTime = System.GetTickCount();
		m_Physics.SetActive(true);
		
		m_pParticle = SCR_ParticleEmitter.CreateAsChild(m_ExhaustParticle, m_Owner, m_ExhaustPosition);
	}
	
	void Setup()
	{
		m_fMassFlowRate = m_fPropellantMass / m_fBurnTime;
		m_fDryMass = m_fStructuralMass + m_fPayloadMass;
		m_fMaxConeAngle = m_fMaxConeAngle * Math.DEG2RAD;
		
		if (m_vExhaustLocation) {
			m_vExhaustLocation.Init(m_Owner);
			vector exhaustTransform[4];
			m_vExhaustLocation.GetLocalTransform(exhaustTransform);
			m_ExhaustPosition = exhaustTransform[3];
		}
			
		if (!m_Physics)
			return;
		
		m_Physics.SetMass(m_fDryMass + m_fPropellantMass);
		
		Launch();
	}
	
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
			
			vector worldThrustPosition = m_Owner.CoordToParent(m_ExhaustPosition);
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
						
			vector worldThrustDirection = m_Owner.VectorToParent(rotatedThrustDirectionXZ);
			worldThrustDirection.Normalize();
			Shape.CreateArrow(worldThrustPosition, m_Owner.CoordToParent(m_ExhaustPosition) + worldThrustDirection*-1, 0.1, COLOR_RED, ShapeFlags.ONCE); 
		
			m_Physics.ApplyForceAt(worldThrustPosition, thrust * worldThrustDirection);
			m_Physics.SetMass(mass);
		}
		
		if (timeUntilBurnout <= 0 && m_pParticle.GetIsPlaying())
			m_pParticle.GetParticles().SetParam(-1, EmitterParam.BIRTH_RATE, 0);
		
		// Calculate Aerodynamics
		float altitude = GetAltitude();
		vector velocity = m_Physics.GetVelocity();
		float rho = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Density);
		float dynamicPressure = 1/2 * rho * velocity.LengthSq();
		
		// Assuming flat plate supersonic newtonian theory
		// https://ntrs.nasa.gov/api/citations/19870002265/downloads/19870002265.pdf
		float alpha = Math.Acos( vector.Dot(velocity.Normalized(), owner.VectorToParent("0 0 1")) );
		float sinAlpha = Math.Sin(alpha);
		float cosAlpha = Math.Cos(alpha);
		float CD = 2 * sinAlpha * sinAlpha * sinAlpha + m_fCF0 * cosAlpha * m_fCD0;	
		float CL = 2 * sinAlpha * sinAlpha * cosAlpha - m_fCF0 * sinAlpha;
		
		vector aeroCenter = m_Owner.CoordToParent(m_aerodynamicCenter);
		float drag = dynamicPressure * m_fDragSurfaceArea * CD;
		m_Physics.ApplyForceAt(aeroCenter, drag * velocity.Normalized() * -1);
		
		float lift = dynamicPressure * m_fLiftSurfaceArea * CL;
		//m_Physics.ApplyForceAt(aeroCenter, lift * m_Owner.CoordToParent("0 1 0"));
		
		Shape.CreateArrow(aeroCenter, aeroCenter + lift * m_Owner.CoordToParent("0 1 0") * 1000, 1, Color.RED, ShapeFlags.NOZBUFFER);
		
		Print(alpha);
		Print(lift);
		
		previousVelocity = m_Physics.GetVelocity();
		
		// Use Z as it is
		vector mat[4];
		owner.GetTransform(mat);
		
		vector x = mat[0];
		x[1] = 0; // remove y component
		x.Normalize(); // renormalize
		
		// calculate x as cross product
		vector z = mat[2];
		vector y = CrossProduct(z, x);
		
		vector newMat[4];
		newMat[0] = x;
		newMat[1] = y;
		newMat[2] = z;
		newMat[3] = mat[3];
		owner.SetTransform(mat);
	}
	
	bool m_ShowDbgUI = true;
	vector previousVelocity = vector.Zero;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
	
		/*float amplitude = m_fMaxConeAngle;
		float angleX = amplitude * Math.Sin(System.GetTickCount() / 1000 * 2 * Math.PI);
		float angleY = amplitude * Math.Cos(System.GetTickCount() / 1000 * 2 * Math.PI);
		SetThrustAngleX(angleX);
		SetThrustAngleY(angleY);
		owner.GetPhysics().SetActive(true);
		Print(angleX);
		Print(angleY);*/
		
		//vector mat2[4];
		//owner.GetTransform(mat2);	
		
		//vector pos = owner.GetOrigin();
		//Shape.CreateArrow(pos, pos + mat2[0] * 2, 0.1, COLOR_RED, ShapeFlags.ONCE);
		//Shape.CreateArrow(pos, pos + mat2[1] * 2, 0.1, COLOR_GREEN, ShapeFlags.ONCE);
		//Shape.CreateArrow(pos, pos + mat2[2] * 2, 0.1, COLOR_BLUE, ShapeFlags.ONCE);
		
#ifdef WORKBENCH
		DbgUI.Begin(string.Format("ISA Properties: %1", owner.GetName()));
		if (m_ShowDbgUI)
		{
			float altitude = GetAltitude();
			float density = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Density);
			float pressure = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Pressure);
			float temperature = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Temperature);
			float dynamicViscosity = ADM_InternationalStandardAtmosphere.GetDynamicViscosity(altitude);
			
			DbgUI.Text(string.Format("Altitude: %1 m", altitude));
			DbgUI.Text(string.Format("Density: %1 kg/m^3", density));
			DbgUI.Text(string.Format("Pressure: %1 Pa", pressure));
			DbgUI.Text(string.Format("Temperature: %1 K", temperature));
			DbgUI.Text(string.Format("Dynamic Viscosity: %1 Pa*s", dynamicViscosity));
			DbgUI.Text("");
		}
		DbgUI.End();
		
		DbgUI.Begin(string.Format("RocketComponent: %1", owner.GetName()));
		if (m_ShowDbgUI)
		{
			Physics ownerPhysics = owner.GetPhysics();
			
			vector vel = m_Physics.GetVelocity();
			vector accVec = (vel - previousVelocity) * 60; // *60 for physics refresh rate
			
			float mach = GetMachNumber();
			float speed = Math.Round(vel.Length() * 100)/100;
			float acc = Math.Round(accVec.Length() * 100)/100;
			DbgUI.Text(string.Format("Speed: %1 mph", speed * 2.23694));
			DbgUI.Text(string.Format("Acc: %1 m/s^2", acc));
			DbgUI.Text(string.Format("Mach Number: %1", Math.Round(mach*100)/100));
			DbgUI.Text("");
		}
		DbgUI.End();
#endif
	}
}