class R3D_BasicAircraftRearmComponentClass: ScriptComponentClass
{
}

class R3D_BasicAircraftRearmComponent : ScriptComponent
{
	[Attribute()]
	protected float m_fRearmTime;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
	}
}