[EntityEditorProps(category: "GameScripted/Aerodynamics", description: "")]
class ADM_EngineComponentClass: ScriptComponentClass
{
}

//! A brief explanation of what this component does.
//! The explanation can be spread across multiple lines.
//! This should help with quickly understanding the script's purpose.
class ADM_EngineComponent : ScriptComponent
{
	[Attribute()]
	protected int m_EngIndex;
	
	[Attribute()]
	protected ref PointInfo m_vThrustInfo;
	
	[Attribute()]
	protected float m_fMaxThrust;
	
	// Minimum throttle before engine cannot sustain operation (e.g combustion engines)
	[Attribute(defvalue: "0", params: "0 1", uiwidget: UIWidgets.Slider)]
	protected float m_fMinThrottle;
	
	// Maximum throttle before engine cannot sustain operation (afterburner)
	[Attribute(defvalue: "1", params: "1 2", uiwidget: UIWidgets.Slider)]
	protected float m_fMaxThrottle;
	
	[Attribute()]
	protected string m_sHitzone;
	
	private vector m_vNozzleExit, m_vExhaustDirection;
	private float m_fThrottle = 0.0;
	private Physics m_Physics = null;
	private SignalsManagerComponent m_SignalManager = null;
	private ADM_AirplaneInput m_input = null;
	private RplComponent m_RplComponent = null;
	private SCR_VehicleDamageManagerComponent m_DamageManager = null;
	private HitZone m_HitZone;
	
	private float m_fRotation = 0;
	private float m_fRPM;
	
	[RplProp()]
	private bool m_bIsEngineOn;
	
	[Attribute()]
	protected float m_fMinRPM;
	
	[Attribute()]
	protected float m_fMaxRPM;
	
	protected int m_iRotationSignal;
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.FRAME | EntityEvent.SIMULATE);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		m_Physics = owner.GetPhysics();
		
		if (m_vThrustInfo)
		{
			m_vThrustInfo.Init(owner);
			
			vector thrustMat[4];
			m_vThrustInfo.GetModelTransform(thrustMat);
			m_vNozzleExit = thrustMat[3];
			m_vExhaustDirection = thrustMat[2];
		}
		
		m_input = ADM_AirplaneInput.Cast(GetOwner().FindComponent(ADM_AirplaneInput));
		m_SignalManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_DamageManager = SCR_VehicleDamageManagerComponent.Cast(owner.FindComponent(SCR_VehicleDamageManagerComponent));
		
		if (m_SignalManager) {
			m_iRotationSignal = m_SignalManager.AddOrFindMPSignal("IntakeRotation", 1, 30, 0, SignalCompressionFunc.RotDEG);
		}
	}
	
	override void EOnInit(IEntity owner)
	{
		m_HitZone = m_DamageManager.GetHitZoneByName(m_sHitzone);
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (m_fThrottle <= 0 || m_fThrottle > 2 || !m_Physics || !m_bIsEngineOn)
			return;
		
		vector thrust = -owner.VectorToParent(m_vExhaustDirection) * m_fMaxThrust * m_fThrottle;
		m_Physics.ApplyImpulseAt(owner.CoordToParent(m_vNozzleExit), thrust * timeSlice);
		//Print(thrust);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (m_input) m_fThrottle = m_input.GetInput(ADM_InputType.Thrust);
		m_fThrottle = Math.Clamp(m_fThrottle, m_fMinThrottle, m_fMaxThrottle);
		
		if (m_bIsEngineOn)
		{
			m_fRPM = m_fMinRPM + (m_fMaxRPM - m_fMinRPM) * m_fThrottle;
			m_fRotation += m_fRPM * 6 * timeSlice; // 1 rpm = 6 deg/s
			m_fRotation = Math.Mod(m_fRotation, 360);
			
			m_SignalManager.SetSignalValue(m_iRotationSignal, m_fRotation);
		}
		
		if (m_HitZone && m_HitZone.GetDamageState() == EDamageState.DESTROYED)
		{
			SetEngineStatus(false);
			Deactivate(owner);
		}
		
		#ifdef WORKBENCH
		DrawDebug(owner);
		#endif
	}
	
	void SetEngineStatus(bool status)
	{
		Rpc(Rpc_Owner_SetEngineStatus, status);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Owner_SetEngineStatus(bool status)
	{
		m_bIsEngineOn = status;
		Replication.BumpMe();
	}
	
#ifdef WORKBENCH
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebug(owner);
	}
#endif
	
	#ifdef WORKBENCH
	void DrawDebug(IEntity owner)
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWPLANEDEBUG))
			return;
		
		if (!m_vThrustInfo)
			return;
		
		vector origin = owner.CoordToParent(m_vNozzleExit);
		Shape.CreateArrow(origin, origin + owner.VectorToParent(m_vExhaustDirection), 0.1, Color.RED, ShapeFlags.ONCE);
	}
	#endif	
}