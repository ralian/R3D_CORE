class R3D_ElevatorComponentClass : R3D_ControlSurfaceComponentClass
{
};

class R3D_ElevatorComponent : R3D_ControlSurfaceComponent
{
	[Attribute(desc: "")]
	float m_UpwardsAngleMax;

	[Attribute(desc: "")]
	float m_DownwardsAngleMax;

	override float UpdateAngle(float dt)
	{
		return R3D_Math.Interpolate(m_Simulation.m_ControlPitch, -1.0, 1.0, -m_DownwardsAngleMax, m_UpwardsAngleMax);
	}
};
