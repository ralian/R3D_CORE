class R3D_FlapComponentClass : R3D_ControlSurfaceComponentClass
{
};

class R3D_FlapComponent : R3D_ControlSurfaceComponent
{
	[Attribute(desc: "")]
	float m_AngleMax;

	override float UpdateAngle(float dt)
	{
		//! TODO
		return 0;
	}
};
