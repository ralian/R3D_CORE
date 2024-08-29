[EntityEditorProps(category: "GameScripted/Aerodynamics", description: "")]
class ADM_GasTurbineEngineComponentClass: ScriptComponentClass
{
}

//! A brief explanation of what this component does.
//! The explanation can be spread across multiple lines.
//! This should help with quickly understanding the script's purpose.
class ADM_GasTurbineEngineComponent : ScriptComponent
{
	[Attribute(params: "1 100000", desc: "Rotational Inertia of shaft that drives compressor and turbine. Units of kilogram*meter^2")]
	protected float m_fShaftInertia; // [kg*m^2]
	
	[Attribute(params: "0 1", desc: "Dynamic friction on shaft", defvalue: "0.02", uiwidget: UIWidgets.Slider)]
	protected float m_fShaftFrictionCoef;
	
	// 2*pi [rad] = 1 [rev]
	// 1 [rad/s] * 1/(2*pi) [rev/rad] * 60 [s/min] -> 9.5493 rpm
	protected const float RS2RPM = 9.5493; // [rad/s -> rpm]
	protected float m_fRPM = 1000;
	
	override event protected void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		ConnectToDiagSystem(owner);
		#endif
		
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	float shaftFriction;
	override event protected void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		// Compressor
		
		
		// Combustor
		
		
		// Turbine
		
		
		// Shaft
		float appliedTorque = 0;	// [N*m]
		shaftFriction = -m_fShaftFrictionCoef * m_fRPM / RS2RPM;
		
		float netShaftTorque = shaftFriction + appliedTorque;
		m_fRPM += (netShaftTorque / m_fShaftInertia) * RS2RPM * timeSlice;
		
		if (Math.AbsFloat(m_fRPM) < 0.1)
			m_fRPM = 0;
		
		// Thrust
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
	
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWENGINEDEBUG))
		{
			DbgUI.Begin(string.Format("ADM_GasTurbineEngineComponent: %1", owner.GetName()));
			DbgUI.Text("");
			DbgUI.Text(string.Format("m_fRPM: %1", m_fRPM));
			DbgUI.Text(string.Format("shaftFriction: %1", shaftFriction));
			DbgUI.PlotLive("Shaft RPM", 600, 350, m_fRPM, 100, 1000);
			DbgUI.Text("");
			DbgUI.End();
		}
#endif
	}
}