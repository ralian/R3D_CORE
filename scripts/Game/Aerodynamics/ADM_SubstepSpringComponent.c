class ADM_SubstepSpringComponentClass: ADM_RigidbodyComponentClass {}
class ADM_SubstepSpringComponent: ADM_RigidbodyComponent
{
	[Attribute(params: "1 inf", defvalue: "10", desc: "N/m")]
	protected float m_fStiffness;
	
	[Attribute(params: "-100 100", defvalue: "1", desc: "m")]
	protected float m_fInitialDisplacement;
	
	[Attribute(params: "1 inf", defvalue: "10", desc: "N*m/rad")]
	protected float m_fRotationalStiffness;
	
	override event protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME);
		
		COM = owner.CoordToParent(m_Physics.GetCenterOfMass());
		m_fNeutralXposition = COM[0] - m_fInitialDisplacement;
	}
	
	protected float m_fNeutralXposition; 
	protected float rotationalDisplacement,displacement = 0.0;
	override void UpdateForcesAndMoments(IEntity owner, float curTime = System.GetTickCount())
	{
		super.UpdateForcesAndMoments(owner);
		
		// spring force
		displacement = COM[0] - m_fNeutralXposition;
		forces += -m_fStiffness*displacement *vector.Right;
			
		// spring moment
		rotationalDisplacement = angles[1];
		moments += -m_fRotationalStiffness*rotationalDisplacement *vector.Right;
	} 
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		#ifdef WORKBENCH
		DbgUI.Begin("Direct Guidance", 0, 0);
		
		DbgUI.Text(string.Format("w: %1", w.Length()));
		DbgUI.PlotLive("w", 500, 200, w.Length(), timeSlice, 1000);
		
		DbgUI.Text(string.Format("displacement: %1", displacement));
		DbgUI.PlotLive("displacement", 500, 200, displacement, timeSlice, 1000);
		
		DbgUI.Text(string.Format("rotationalDisplacement: %1", rotationalDisplacement));
		DbgUI.PlotLive("rotationalDisplacement", 500, 200, rotationalDisplacement, timeSlice, 1000);
		
		DbgUI.End();
		#endif
	}
}