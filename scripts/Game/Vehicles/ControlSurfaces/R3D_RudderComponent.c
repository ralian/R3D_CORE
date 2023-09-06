class R3D_RudderComponentClass : R3D_ControlSurfaceComponentClass
{
};

class R3D_RudderComponent : R3D_ControlSurfaceComponent
{
	[Attribute(desc: "")]
	float m_AngleMax;

	override vector GetTypicalForceApplicationDirection()
	{
		return "1 0 0";
	}

	override float UpdateAngle(float dt)
	{
		return -m_Simulation.m_ControlYaw * m_AngleMax;
	}
};
