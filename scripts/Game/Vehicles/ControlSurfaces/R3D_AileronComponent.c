class R3D_AileronComponentClass : R3D_ControlSurfaceComponentClass
{
};

class R3D_AileronComponent : R3D_ControlSurfaceComponent
{
	[Attribute(desc: "")]
	float m_UpwardsAngleMax;

	[Attribute(desc: "")]
	float m_DownwardsAngleMax;
	
	override float UpdateAngle(float dt)
	{
		float x = m_Position[0];
		return R3D_Math.Interpolate(-x.Sign() * m_Simulation.m_ControlRoll, -1.0, 1.0, -m_DownwardsAngleMax, m_UpwardsAngleMax);
	}
};
