class R3D_ControlSurfaceComponentClass : R3D_VehiclePartBaseComponentClass
{
};

class R3D_ControlSurfaceComponent : R3D_VehiclePartBaseComponent
{
	[Attribute()]
	float m_StallAngle;

	[Attribute("", UIWidgets.ComboBox, "If not set, the center is calculated from the points")]
	string m_Point_Center;

	[Attribute("", UIWidgets.ComboBox, "Forward")]
	string m_Point_00;

	[Attribute("", UIWidgets.ComboBox, "Forward")]
	string m_Point_01;

	[Attribute("", UIWidgets.ComboBox, "Backward")]
	string m_Point_10;

	[Attribute("", UIWidgets.ComboBox, "Backward")]
	string m_Point_11;

	vector m_TypicalForceApplicationDirection;

	vector m_AirFlowNormal;
	float m_AirflowMagnitudeSq;

	float m_AngleOfAttack;

	bool m_Stalling;

	float m_Camber;
	float m_Area;
	vector m_Position;

	float m_LiftAngle;
	float m_DragAngle;

	float m_LiftCoef;
	float m_DragCoef;
	float m_PressureCoef;

	float m_Angle;

	override bool OnInitialize(R3D_VehicleSimulationComponent simulation)
	{
		if (!super.OnInitialize(simulation))
		{
			return false;
		}

		if (GetGame().InPlayMode())
		{
			if (m_Point_00 == string.Empty)
			{
				Debug.Error("Point_00 not set");
				return false;
			}

			if (m_Point_01 == string.Empty)
			{
				Debug.Error("Point_01 not set");
				return false;
			}

			if (m_Point_10 == string.Empty)
			{
				Debug.Error("Point_10 not set");
				return false;
			}

			if (m_Point_11 == string.Empty)
			{
				Debug.Error("Point_11 not set");
				return false;
			}
		}

		m_TypicalForceApplicationDirection = GetTypicalForceApplicationDirection();

		vector p00 = GetBonePosition(m_Point_00);
		vector p01 = GetBonePosition(m_Point_01);
		vector p10 = GetBonePosition(m_Point_10);
		vector p11 = GetBonePosition(m_Point_11);

		//! Get the depth direction of the aerofoil to calculate the curve
		vector d0 = vector.Direction(p00, p10);
		vector d1 = vector.Direction(p01, p11);

		//! Depths of aerofoil
		float y0 = d0.NormalizeSize();
		float y1 = d1.NormalizeSize();

		//! Widths of aerofoil
		float x0 = vector.Distance(p00, p01);
		float x1 = vector.Distance(p10, p11);

		//! Area of both minimal and maximal size of aerofoil
		float a0 = Math.Max(y0, y1) * Math.Max(x0, x1);
		float a1 = Math.Min(y0, y1) * Math.Min(x0, x1);

		//! Camber at two points
		float c0 = Math.Asin(vector.Dot("0 0 1", d0));
		float c1 = Math.Asin(vector.Dot("0 0 1", d1));

		//! The position where the force will be applied on the plane
		if (m_Point_Center != string.Empty)
		{
			m_Position = GetBonePosition(m_Point_Center);
		}
		else
		{
			vector p = p00 + p01 + p10 + p11;
			m_Position = p * 0.25;
		}

		//! Average the area between maximal and minimal
		m_Area = (a0 + a1) * 0.5;

		//! Get the maximum curve of the aerofoil
		m_Camber = Math.Max(c0, c1);

		return true;
	}

	float UpdateAngle(float dt)
	{
		return 0;
	}

	vector GetPosition()
	{
		return m_Position;
	}

	vector GetTypicalForceApplicationDirection()
	{
		return "0 1 0";
	}

	override void OnCompute(float dt, Physics physics)
	{
		m_AirFlowNormal = -m_Simulation.GetModelVelocityAt(m_Position);
		m_AirflowMagnitudeSq = m_AirFlowNormal.NormalizeSize();
		m_AirflowMagnitudeSq *= m_AirflowMagnitudeSq;

		if (Math.AbsFloat(m_AirflowMagnitudeSq) < 0.0001)
		{
			return;
		}

		m_Angle = UpdateAngle(dt) + m_Camber;

		m_AngleOfAttack = Math.Asin(vector.Dot(m_TypicalForceApplicationDirection, m_AirFlowNormal)) * Math.RAD2DEG;

		m_PressureCoef = m_Area * 0.5 * m_Simulation.GetDensity() * m_AirflowMagnitudeSq;

		m_LiftAngle = m_AngleOfAttack + m_Angle;

		m_LiftCoef = 0.0;
		if (Math.AbsFloat(m_LiftAngle) <= (m_StallAngle * 2.0))
		{
			m_LiftCoef = Math.Sin(m_LiftAngle * (Math.PI * 0.5) / m_StallAngle) * 2.0;
		}

		m_DragAngle = m_AngleOfAttack;
		if (m_DragAngle > 90.0)
		{
			m_DragAngle = 180.0 - m_DragAngle;
		}
		else if (m_DragAngle < -90.0)
		{
			m_DragAngle = -180.0 - m_DragAngle;
		}

		m_DragCoef = (m_DragAngle + m_Angle) / (m_StallAngle + Math.AbsFloat(m_Angle));
		m_DragCoef = 0.05 + (m_DragCoef * m_DragCoef);
	}

	override void OnSimulate(float dt, Physics physics)
	{
		if (Math.AbsFloat(m_AirflowMagnitudeSq) < 0.0001)
		{
			return;
		}
		
		float mass = physics.GetMass();
		float invMass = 1.0 / mass;
		float dtInvMass = invMass / dt;

		vector force = (m_TypicalForceApplicationDirection * m_PressureCoef * m_LiftCoef) + (m_AirFlowNormal * m_PressureCoef * m_DragCoef);
		vector impulse = force * dt;

		vector position = GetPosition();
		
#ifdef WORKBENCH
		m_Simulation.Debug_Add(R3D_DebugShape.CreateDirLine(0xFFFFFF00, ShapeFlags.NOZBUFFER, position, impulse * dtInvMass));

		m_Simulation.Debug_Add(R3D_DebugShape.CreateSphere(0xAA00FF00, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, position, 0.1));
#endif

		// Convert to world space
		m_Simulation.VMultiply4(position, position);
		m_Simulation.VMultiply3(impulse, impulse);

#ifdef WORKBENCH
		if (m_Simulation.m_DebugSimulateAerofoils)
		{
#endif

		// Apply the impulse
		physics.ApplyImpulseAt(position, impulse);

#ifdef WORKBENCH
		}
#endif
	}
};
