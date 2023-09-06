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
	
	private vector m_vNozzleExit, m_vExhaustDirection;
	private float m_fThrottle = 0.0;
	private Physics m_Physics = null;
	private SignalsManagerComponent m_SignalManager = null;
	private ADM_AirplaneInput m_input = null;
	private RplComponent m_RplComponent = null;
	private float m_fRotation = 0;
	private float m_fRPM;
	
	//[RplProp()]
	private bool m_bIsEngineOn = false;
	
	[Attribute()]
	protected float m_fMinRPM;
	
	[Attribute()]
	protected float m_fMaxRPM;
	
	//protected bool m_Fire = false, m_On = false; // both todo
	//protected float m_MassFlow = 0, m_RPM = 0, m_ABPct = 0;
	//protected int m_SigMassFlow, m_SigRPM, m_SigABPct;
	//protected int m_SigRPM;
	
	protected int m_iRotationSignal;
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.FRAME | EntityEvent.SIMULATE);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		m_Physics = owner.GetPhysics();
		
		m_vThrustInfo.Init(owner);
		
		vector thrustMat[4];
		m_vThrustInfo.GetModelTransform(thrustMat);
		m_vNozzleExit = thrustMat[3];
		m_vExhaustDirection = thrustMat[2];
		
		m_input = ADM_AirplaneInput.Cast(GetOwner().FindComponent(ADM_AirplaneInput));
		m_SignalManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (m_SignalManager) {
			m_iRotationSignal = m_SignalManager.AddOrFindMPSignal("IntakeRotation", 1, 30, 0, SignalCompressionFunc.RotDEG);
			//m_SigMassFlow = m_sigs.AddOrFindSignal("R3D_MassFlow_" + m_EngIndex);
			//m_SigABPct = m_sigs.AddOrFindSignal("R3D_ABPct_" + m_EngIndex);
			//m_SigRPM = m_sigs.AddOrFindMPSignal("RPM_" + m_EngIndex, 0.01, 30, 0, SignalCompressionFunc.Range01);
		}
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (m_fThrottle <= 0 || m_fThrottle > 2 || !m_Physics || !m_bIsEngineOn || !m_RplComponent.IsOwner())
			return;
		
		vector thrust = -owner.VectorToParent(m_vExhaustDirection) * m_fMaxThrust * m_fThrottle;
		m_Physics.ApplyImpulseAt(owner.CoordToParent(m_vNozzleExit), thrust * timeSlice);
	}
	
	//const float RPMThrottleTimeConstant = 2.0;
	//const float RPMThrottleRatio = 10000.0; // This should be optimized with a power of 2 constant
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
			
		//float rpmDiff = (m_fThrottle * RPMThrottleRatio) - m_RPM;
		//float rpmDelta = rpmDiff * timeSlice / RPMThrottleTimeConstant;
		//m_RPM += rpmDelta;
		
		//if (m_sigs) {
		//	m_sigs.SetSignalValue(m_SigRPM, m_fThrottle); 
		//}
		
		#ifdef WORKBENCH
		DrawDebug(owner);
		#endif
	}
	
	void SetEngineStatus(bool status)
	{
		m_bIsEngineOn = status;
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
		if (!m_vThrustInfo)
			return;
		
		vector origin = owner.CoordToParent(m_vNozzleExit);
		Shape.CreateArrow(origin, origin + owner.VectorToParent(m_vExhaustDirection), 0.1, Color.RED, ShapeFlags.ONCE);
	}
	#endif	
}