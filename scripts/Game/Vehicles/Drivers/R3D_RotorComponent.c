class R3D_RotorComponentClass : R3D_VehiclePartBaseComponentClass
{
};

class R3D_RotorComponent : R3D_VehiclePartBaseComponent
{
	[Attribute()]
	vector m_Position;

	[Attribute()]
	vector m_Direction;

	[Attribute()]
	float m_MaxThrust;

	//float m_Velocity;

	override void OnCompute(float dt, Physics physics)
	{
	}

	override void OnSimulate(float dt, Physics physics)
	{
		/*
		//! Constants
		float NumBlades = 4.0;
		float Radius = 1.0;
		float BladeWidth = 0.4;
		float BladePitchAngle = 2.0 * Math.DEG2RAD;
		float BladeArea = Radius * (Math.PI * BladeWidth);
		float RotorArea = BladeArea * NumBlades;

		//! From m_Simulation
		float velocity = 0;
		float density = 0;

		//! Functions
		float SpeedRatio = m_Velocity * Radius / velocity;




		float PowerCoefficient = 0.5 * (BladePitchAngle)
		*/


		vector force = m_Direction * m_Simulation.m_ControlThrust * m_MaxThrust;

		vector position = m_Position;
		vector impulse = force * dt;

		// Convert to world space
		m_Simulation.VMultiply4(position, position);
		m_Simulation.VMultiply3(impulse, impulse);

		// Apply the impulse
		physics.ApplyImpulseAt(position, impulse);
	}

};
