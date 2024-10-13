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
	
	//protected IEntity m_Owner;
	
	protected float m_fLaunchTime = -1;	
	protected float m_fMassFlowRate;
	protected float m_fDryMass;
	protected vector m_ExhaustPosition;
	
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
		return owner.GetOrigin()[1];
	}
	
	float GetTimeUntilBurnout(float curTime = System.GetTickCount())
	{
		float timeSinceLaunch = (curTime - m_fLaunchTime)/1000; // [s]
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
		
		m_fLaunchTime = System.GetTickCount();
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
			m_ExhaustPosition = exhaustTransform[3];
		}
			
		if (!m_Physics)
			return;
		
		m_Physics.SetMass(m_fDryMass + m_fPropellantMass);
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
	
	override event void OnPostInit(IEntity owner)
	{
		Setup(owner);
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME);
		
		Launch();
	}
	
	float gravityGradientConstant = 3*5.972*6.67430*Math.Pow(10,13)/6378000/6378000/6378000; // G*M/R^3; gravitational constant * earth mass / (distance to earth center)^3
	override void UpdateForcesAndMoments(IEntity owner, float curTime = System.GetTickCount())
	{
		super.UpdateForcesAndMoments(owner);
		
		// gravity
		forces += m_fMass * Physics.VGravity;
		
		// gravity-gradient torque, negligible.
		//vector gravityGradient = gravityGradientConstant*"0 0 1"*(m_vInertia*"0 0 1");
		//moments += gravityGradient;
		//Print(gravityGradient);
		
		// thrust
		float timeUntilBurnout = GetTimeUntilBurnout(curTime);
		if (timeUntilBurnout >= 0) {
			float mass = m_fDryMass + m_fPropellantMass*timeUntilBurnout/m_fBurnTime;
			vector thrust = m_fIsp * m_fMassFlowRate * 9.81 * "0 0 1"; // Isp = T/mdot/g_e
		
			forces += owner.VectorToParent(thrust);
			moments += m_ExhaustPosition * thrust;
			m_fMass = mass;
		}
		
		moments += "10 20 100";
		
	} 
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		//float timeUntilBurnout = GetTimeUntilBurnout();
		//if (timeUntilBurnout <= 0 && m_pParticle.GetIsPlaying())
		//	m_pParticle.GetParticles().SetParam(-1, EmitterParam.BIRTH_RATE, 0);
		
		DbgUI.Begin("Rocket Move Component", 0, 0);
		
		DbgUI.Text(string.Format("m_fMass: %1 kg", Math.Round(m_fMass*100)/100));
		DbgUI.PlotLive("m_fMass", 500, 200, m_fMass, timeSlice, 1000);
		
		DbgUI.Text(string.Format("Y: %1 m", Math.Round(COM[1]*100)/100));
		DbgUI.PlotLive("Y", 500, 200, Math.Round(COM[1]*100)/100, timeSlice, 1000);
		
		DbgUI.Text(string.Format("vy: %1 m/s", Math.Round(v[1]*100)/100));
		DbgUI.PlotLive("vy", 500, 200, Math.Round(v[1]*100)/100, timeSlice, 1000);
		
		DbgUI.End();
		
		vector mat2[4];
		owner.GetTransform(mat2);	

		vector pos = owner.GetOrigin();
		Shape.CreateArrow(pos, pos + mat2[0] * 2, 0.1, COLOR_RED, ShapeFlags.ONCE);
		Shape.CreateArrow(pos, pos + mat2[1] * 2, 0.1, COLOR_GREEN, ShapeFlags.ONCE);
		Shape.CreateArrow(pos, pos + mat2[2] * 2, 0.1, COLOR_BLUE, ShapeFlags.ONCE);
		
	}
	
	/*vector worldThrustPosition;
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
		
		vector mat2[4];
		owner.GetTransform(mat2);	
		
		vector pos = owner.GetOrigin();
		Shape.CreateArrow(pos, pos + mat2[0] * 2, 0.1, COLOR_RED, ShapeFlags.ONCE);
		Shape.CreateArrow(pos, pos + mat2[1] * 2, 0.1, COLOR_GREEN, ShapeFlags.ONCE);
		Shape.CreateArrow(pos, pos + mat2[2] * 2, 0.1, COLOR_BLUE, ShapeFlags.ONCE);
		
		Shape.CreateArrow(worldThrustPosition, m_Owner.CoordToParent(m_ExhaustPosition) + worldThrustDirection*-1, 0.1, COLOR_RED, ShapeFlags.ONCE); 
		
#ifdef WORKBENCH		
		DbgUI.Begin(string.Format("RocketComponent: %1", owner.GetName()));
		if (m_ShowDbgUI && m_Physics)
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
	}*/
}