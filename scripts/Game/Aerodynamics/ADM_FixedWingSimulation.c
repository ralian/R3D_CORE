[EntityEditorProps(category: "GameScripted/Aerodynamics", description: "")]
class ADM_FixedWingSimulationClass: ScriptGameComponentClass
{
}

class ADM_FixedWingSimulation : ScriptGameComponent
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, "Contains wing sections which form the entire wing", category: "Fixed Wing Simulation")]
	ref array<ref ADM_Wing> m_Wings;
	
	[Attribute(category: "Fixed Wing Simulation", desc: "Offset for aerodynamic center. For drag calculation.")]
	protected vector m_vAerodynamicCenterOffset;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fFrontalDragArea;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fFrontalDragCoefficient;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fSideDragArea;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fSideDragCoefficient;
	
	[Attribute(uiwidget: UIWidgets.Object, category: "Fixed Wing Simulation")]
	ref array<ref ADM_LandingGear> m_Gear;
	
	[Attribute(category: "Fixed Wing Simulation")]
	protected string m_sHullHitzone;
	
	[RplProp()]
	protected bool m_bIsEngineOn = false; //Set the engine to be off by default
	
	[RplProp()]
	protected bool m_bGearState = true; // true = deployed, false = retracted
	
	// Required Component References
	private IEntity m_Owner = null;
	private Physics m_Physics = null;
	private ADM_AirplaneInput m_Input = null;
	private vector m_vAerodynamicCenter;
	private SignalsManagerComponent m_SignalsManager = null;
	private VehicleBaseSimulation m_VehicleBaseSim = null;
	private RplComponent m_RplComponent = null;
	private SCR_PlayerController m_LocalPlayerController = null;
	private CameraHandlerComponent m_LocalCameraHandler = null;
	private CameraManager m_CameraManager = null;
	private ref array<ADM_EngineComponent> m_Engines = {}; 
	private SCR_VehicleDamageManagerComponent m_DamageManager = null;
	private HitZone m_HullHitZone = null;
	private NwkCarMovementComponent m_NwkMovement = null;
	
	// Animation signals
	private int m_iPitchSignal = -1;
	private int m_iRollSignal = -1;
	private int m_iAltitudeSignal = -1;
	private int m_iClimbRateSignal = -1;
	private int m_iHeadingSignal = -1;
	private int m_iAccSignal = -1;
	private int m_iAirspeedSignal = -1;
	private int m_iFlapsNeedleSignal = -1;
	private int m_iSpeedSignal = -1;
	private int m_iRPMSignal = -1;
	private int m_iAircraftIsEngineOnSignal = -1;
	private int m_iThirdPersonSignal = -1;
	private int m_iFirstPersonSignal = -1;
	private int m_iViewAngleSignal = -1;
	
	// Hand IK
	protected CharacterAnimationComponent m_CharacterAnim = null;
	protected IEntity m_Pilot;
	protected int m_iCharacterAileronInput = -1;
	protected int m_iCharacterElevatorInput = -1;
	protected int m_iCharacterThrottleInput = -1;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.SIMULATE | EntityEvent.POSTFRAME);
		ConnectToDiagSystem(owner);
		
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (ev)
		{
			ev.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered);
			ev.RegisterScriptHandler("OnCompartmentLeft", this, OnCompartmentLeft);
		}
		
		// Required Components
		m_Owner = owner;
		m_Physics = owner.GetPhysics();
		m_Input = ADM_AirplaneInput.Cast(owner.FindComponent(ADM_AirplaneInput));
		m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_VehicleBaseSim = VehicleBaseSimulation.Cast(owner.FindComponent(VehicleBaseSimulation));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_CameraManager = GetGame().GetCameraManager();
		m_DamageManager = SCR_VehicleDamageManagerComponent.Cast(owner.FindComponent(SCR_VehicleDamageManagerComponent));
		m_NwkMovement = NwkCarMovementComponent.Cast(owner.FindComponent(NwkCarMovementComponent));
		m_Physics.SetActive(true);
		
		// Find all engines
		array<Managed> components = {};
		owner.FindComponents(ADM_EngineComponent, components);
		foreach (Managed cmp : components)
		{
			ADM_EngineComponent engCmp = ADM_EngineComponent.Cast(cmp);
			if (engCmp)
			{
				m_Engines.Insert(engCmp);
			}
		}
		
		// Signals
		if (m_SignalsManager)
		{
			// Instruments
			m_iPitchSignal = m_SignalsManager.AddOrFindSignal("Pitch");
			m_iRollSignal = m_SignalsManager.AddOrFindSignal("Roll");
			m_iAltitudeSignal = m_SignalsManager.AddOrFindSignal("Altitude");
			m_iClimbRateSignal = m_SignalsManager.AddOrFindSignal("ClimbRate");
			m_iHeadingSignal = m_SignalsManager.AddOrFindSignal("Heading");
			m_iAccSignal = m_SignalsManager.AddOrFindSignal("AccelerationG");
			m_iAirspeedSignal = m_SignalsManager.AddOrFindSignal("Airspeed");
			m_iFlapsNeedleSignal = m_SignalsManager.AddOrFindSignal("FlapsNeedle");
			
			// System stuff
			m_iRPMSignal = m_SignalsManager.AddOrFindMPSignal("RPM", 1, 30, 0, SignalCompressionFunc.Range01);
			m_iSpeedSignal = m_SignalsManager.AddOrFindMPSignal("Speed", 1, 30, 0, SignalCompressionFunc.None);
			m_iAircraftIsEngineOnSignal = m_SignalsManager.AddOrFindMPSignal("AircraftIsEngineOn", 0.1, 30, 1, SignalCompressionFunc.Range01);
			m_iThirdPersonSignal = m_SignalsManager.AddOrFindSignal("AircraftThirdPersonAndEngine", 1);
			m_iFirstPersonSignal = m_SignalsManager.AddOrFindSignal("AircraftFirstPersonAndEngine", 1);
			m_iViewAngleSignal = m_SignalsManager.AddOrFindSignal("AircraftViewAngle", 1);
			
			m_SignalsManager.SetSignalValue(m_iAircraftIsEngineOnSignal, m_bIsEngineOn);
			m_SignalsManager.SetSignalValue(m_iThirdPersonSignal, (bool)1 && m_bIsEngineOn);
			m_SignalsManager.SetSignalValue(m_iThirdPersonSignal, (bool)0 && m_bIsEngineOn);
		}
		
		foreach (ADM_LandingGear gear: m_Gear)
		{
			gear.m_vPosition.Init(owner);
		}
		
		CalculatePanels();
	}
	
	override void EOnInit(IEntity owner)
	{		
		m_HullHitZone = m_DamageManager.GetHitZoneByName(m_sHullHitzone);
	}
	
	void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		m_CharacterAnim = CharacterAnimationComponent.Cast(occupant.FindComponent(CharacterAnimationComponent));
		if (m_CharacterAnim)
		{
			m_iCharacterAileronInput = m_CharacterAnim.BindVariableFloat("AircraftAileronInput");
			m_iCharacterElevatorInput = m_CharacterAnim.BindVariableFloat("AircraftElevatorInput");
			m_iCharacterThrottleInput = m_CharacterAnim.BindVariableFloat("AircraftThrottleInput");
		}
		
		m_Pilot = occupant;
		m_Owner.GetPhysics().SetActive(true);
		
		if (m_NwkMovement && m_RplComponent)
		{
			if(!m_RplComponent.IsProxy())
			{
				m_NwkMovement.SetAllowance(false, 100, 100, 100, 100);
			} else if(m_RplComponent.IsProxy() && m_RplComponent.IsOwner()) {
				m_NwkMovement.SetPrediction(false);
			} 
		}
	}
	
	void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{		
		m_CharacterAnim = null;	
		m_iCharacterAileronInput = -1;
		m_iCharacterElevatorInput = -1;
		m_iCharacterThrottleInput = -1;
		m_Pilot = null;
	}
	
	bool IsEngineOn()
	{
		return m_bIsEngineOn;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Server_ToggleEngine()
	{
		m_bIsEngineOn = !m_bIsEngineOn;
		Replication.BumpMe();
		
		m_SignalsManager.SetSignalValue(m_iAircraftIsEngineOnSignal, m_bIsEngineOn);
		foreach(ADM_EngineComponent engine : m_Engines)
		{
			engine.SetEngineStatus(m_bIsEngineOn);
		}
	}
	
	float GetAltitude()
	{
		return m_Owner.GetOrigin()[1];
	}
	
	float GetMachNumber()
	{
		float speedOfSound = ADM_InternationalStandardAtmosphere.GetValue(GetAltitude(), ADM_ISAProperties.SpeedOfSound);
		vector vel = m_Physics.GetVelocity();
		float mach = vel.Length() / speedOfSound;
		
		return mach;
	}
	
	void ToggleGear()
	{
		Rpc(Rpc_Owner_ToggleGear);	
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Owner_ToggleGear()
	{
		m_bGearState = !m_bGearState;
		
		if (m_SignalsManager)
		{
			int signalIndex = m_SignalsManager.AddOrFindMPSignal("ToggleGear", 0, 30, (int)(!m_bGearState), SignalCompressionFunc.RotDEG);
			m_SignalsManager.SetSignalValue(signalIndex, (int)(!m_bGearState));
		}
		
		if (!m_bGearState)
		{
			m_VehicleBaseSim.Deactivate(m_Owner);
			m_Physics.EnableGravity(true);
		} else {
			m_VehicleBaseSim.Activate(m_Owner);
		}
			
		Replication.BumpMe();
	}
	
	void FindLocalPlayerController()
	{
		m_LocalPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!m_LocalPlayerController) return;
		
		m_LocalCameraHandler = CameraHandlerComponent.Cast(m_LocalPlayerController.GetMainEntity().FindComponent(CameraHandlerComponent));
		if (!m_LocalCameraHandler) return;
	}
	
	protected TimeAndWeatherManagerEntity timeManager;
	protected ChimeraWorld world;
	vector GetWindVector()
	{
		if (!world)
		{
			world = m_Owner.GetWorld();
		}
		
		if (!timeManager)
		{
			timeManager = world.GetTimeAndWeatherManager();
			if (!timeManager) 
				return vector.Zero;
		}
		
		vector angles = vector.Zero;
		float speed = timeManager.GetWindSpeed();
		angles[0] = timeManager.GetWindDirection() + 180; // GM wind is weird, also the clouds move the opposite direction of the wind vector??
		
		vector mat[3];
		Math3D.AnglesToMatrix(angles, mat);
		
		return speed * mat[2];
	}
	
	override event protected bool OnTicksOnRemoteProxy() 
	{ 
		return true; 
	}
	
	void OwnerTick(IEntity owner, float timeSlice)
	{
		foreach (ADM_LandingGear gear: m_Gear)
		{
			if (m_bGearState && gear.GetState() < 1)
			{
				gear.RotateGear(gear.m_fRotationRate * timeSlice);
			}
			
			if (!m_bGearState && gear.GetState() > 0)
			{
				gear.RotateGear(-gear.m_fRotationRate * timeSlice);
			}
			
			if (m_SignalsManager)
			{
				if (gear.m_iSignalIndex == -1)
				{
					gear.m_iSignalIndex = m_SignalsManager.AddOrFindMPSignal(gear.m_sSignal, 0.05, 30, 0, SignalCompressionFunc.RotDEG);
				}
				
				float signal = (1 - gear.GetState()) * gear.m_fRotationAngle;
				m_SignalsManager.SetSignalValue(gear.m_iSignalIndex, signal);
			}
		}
		
		if (m_HullHitZone && m_HullHitZone.GetDamageState() == EDamageState.DESTROYED)
		{
			if (m_bIsEngineOn)
			{
				Rpc(Rpc_Server_ToggleEngine);
			}
			
			Deactivate(m_Owner);
		}
	}
	
	void ProxyTick(IEntity owner, float timeSlice)
	{
		if (!m_LocalPlayerController) FindLocalPlayerController();
		if (m_LocalCameraHandler && m_Input && m_SignalsManager)
		{
			int thirdPerson = m_Input.IsControlActive() && m_LocalCameraHandler.IsInThirdPerson();
			if (!m_Input.IsControlActive()) thirdPerson = 1;
			
			m_SignalsManager.SetSignalValue(m_iThirdPersonSignal, thirdPerson && m_bIsEngineOn);
			m_SignalsManager.SetSignalValue(m_iFirstPersonSignal, !thirdPerson && m_bIsEngineOn);
		}
		
		if (m_CameraManager && m_CameraManager.CurrentCamera())
		{
			vector camMat[4];
			m_CameraManager.CurrentCamera().GetTransform(camMat);
			
			vector planeForward = m_Owner.VectorToParent(vector.Forward);
			vector planeToCamera = camMat[3] - m_Owner.GetOrigin();
			planeToCamera.Normalize();
			
			float n_angle = 1 - 0.5 * Math.AbsFloat(vector.Dot(planeForward, planeToCamera) - 1);
			m_SignalsManager.SetSignalValue(m_iViewAngleSignal, n_angle);
		}
		
		// Cockpit Animation Signals
		vector velocity = m_Physics.GetVelocity();
		vector flowVelocity = velocity + GetWindVector();
		if (m_SignalsManager)
		{
			vector angles = m_Owner.GetAngles();
			float climbRate = m_Physics.GetVelocity()[1];
			
			m_SignalsManager.SetSignalValue(m_iPitchSignal, angles[0]);
			m_SignalsManager.SetSignalValue(m_iRollSignal, angles[2]);
			m_SignalsManager.SetSignalValue(m_iAltitudeSignal, GetAltitude());
			m_SignalsManager.SetSignalValue(m_iClimbRateSignal, climbRate);
			m_SignalsManager.SetSignalValue(m_iHeadingSignal, angles[1]);
			m_SignalsManager.SetSignalValue(m_iAccSignal, m_VehicleBaseSim.GetGForceMagnitude());
			m_SignalsManager.SetSignalValue(m_iAirspeedSignal, flowVelocity.Length());
			m_SignalsManager.SetSignalValue(m_iFlapsNeedleSignal, m_Input.GetInput(ADM_InputType.Flap));
			m_SignalsManager.SetSignalValue(m_iRPMSignal, m_Input.GetInput(ADM_InputType.Thrust));
		}
		
		if (m_CharacterAnim)
		{
			m_CharacterAnim.SetVariableFloat(m_iCharacterAileronInput, m_Input.GetInput(ADM_InputType.Aileron));
			m_CharacterAnim.SetVariableFloat(m_iCharacterElevatorInput, m_Input.GetInput(ADM_InputType.Elevator));
			m_CharacterAnim.SetVariableFloat(m_iCharacterThrottleInput, m_Input.GetInput(ADM_InputType.Thrust));
		}
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		ProxyTick(owner, timeSlice);
		if (m_RplComponent.IsOwner())
		{
			OwnerTick(owner, timeSlice);
		}
	}
	
	private float lastSendTime = -50000;
	private float updateFrequency = 5;
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (!m_RplComponent.IsOwner())
			return;
		
		vector mat[4];
		m_Owner.GetTransform(mat);
		
		float curTime = System.GetTickCount();
		if ((curTime - lastSendTime) > 1000/updateFrequency)
		{
			Rpc(Rpc_Server_ReceiveNewStates, mat, m_Physics.GetVelocity(), m_Physics.GetAngularVelocity());
			lastSendTime = curTime;
		}
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void Rpc_Server_ReceiveNewStates(vector mat[4], vector velocity, vector angularVelocity)
	{
		m_Owner.SetTransform(mat);
		m_Physics.SetVelocity(velocity);	
		m_Physics.SetAngularVelocity(angularVelocity);
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		super.EOnSimulate(owner, timeSlice);
		
		if (!m_Physics || !m_Physics.IsActive())
			return;
		
		vector com = owner.CoordToParent(m_Physics.GetCenterOfMass());
		vector coa = owner.CoordToParent(m_vAerodynamicCenter + m_vAerodynamicCenterOffset);
		
		vector wind = GetWindVector();
		vector absoluteVelocity = m_Physics.GetVelocity();
		vector flowVelocity = absoluteVelocity + wind;
		vector flowVelocityLocal = owner.VectorToLocal(flowVelocity);
		
		float altitude = GetAltitude();
		float density = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Density); // [kg/m^3]
		float dynamicPressure = 1/2 * density * flowVelocity.LengthSq(); // [Pa]
		
		for (int i = 0; i < m_Wings.Count(); i++)
		{
			float aspectRatio = m_Wings[i].GetAspectRatio();	
			for (int j = 0; j < m_Wings[i].m_Sections.Count(); j++)
			{
				ADM_WingSection curSection = m_Wings[i].m_Sections[j];
				
				float surfaceArea = curSection.m_fSurfaceArea;
				vector vSpan = owner.VectorToParent(curSection.m_vSpan);
				vector aerocenter = owner.CoordToParent(curSection.m_vAerodynamicCenter);
											
				vector sectionFlowVelocity = m_Physics.GetVelocityAt(aerocenter) + wind;
				vector sectionFlowVelocityLocal = m_Owner.VectorToLocal(sectionFlowVelocity);
				float sectionDynamicPressure = 1/2 * density * sectionFlowVelocity.LengthSq(); // [Pa]
				
				vector dragDir = sectionFlowVelocity.Normalized();
				vector liftDir = dragDir * vSpan;
				
				float angleOfAttack = Math.Atan2(-sectionFlowVelocityLocal[1], sectionFlowVelocityLocal[2])*Math.RAD2DEG;
				
				float CL = 0;
				float CD = 0;
				
				if (curSection.m_ControlSurfaces && curSection.m_ControlSurfaces.Count() > 0)
				{
					foreach(ADM_ControlSurface controlSurface: curSection.m_ControlSurfaces)
					{
						float deflectionAngle = controlSurface.GetAngle(m_Input.GetInput(controlSurface.m_Type));
					
						// Control surfaces effect to wing section
						angleOfAttack += controlSurface.GetDeltaAoA(curSection, deflectionAngle, angleOfAttack);
						CL += controlSurface.GetDeltaCL(curSection, deflectionAngle, angleOfAttack);
						CD += controlSurface.GetDeltaCD(curSection, deflectionAngle, angleOfAttack);
						
						if (m_SignalsManager)
						{
							if (controlSurface.m_iSignalIndex == -1)
							{
								controlSurface.m_iSignalIndex = m_SignalsManager.AddOrFindMPSignal(controlSurface.m_sSignal, 0.1, 30, 0, SignalCompressionFunc.RotDEG);
							}
							m_SignalsManager.SetSignalValue(controlSurface.m_iSignalIndex, deflectionAngle);
						}
					}
				}
				
				float dist = (angleOfAttack + 90) / 180;
				CL += Math3D.Curve(ECurveType.CurveProperty2D, dist, curSection.m_vLiftCurve)[1] * (aspectRatio / (aspectRatio + 2));
				CD += Math3D.Curve(ECurveType.CurveProperty2D, dist, curSection.m_vDragCurve)[1];
				
				// Prandtl-Glauert compressibility factor
				float speedOfSound = ADM_InternationalStandardAtmosphere.GetValue(GetAltitude(), ADM_ISAProperties.SpeedOfSound);
				float machSqr = flowVelocity.LengthSq() / speedOfSound / speedOfSound;
				
				// at mach 0.3 (sqrt = 0.0625) compressibility effects are greater than 5% error, in the transonic-supersonic M > 0.75 (sqrt = 0.5625) regime this correction is not valid
				if (machSqr > 0.0625 && machSqr < 0.5625) 
					CL = CL / Math.Sqrt(1 - machSqr);
				
				vector vLift = liftDir * sectionDynamicPressure * CL * surfaceArea; // [N]
				vector vDrag = -dragDir * (sectionDynamicPressure * CD * surfaceArea); // [N]
				
				m_Physics.ApplyImpulseAt(aerocenter, vLift * timeSlice);
				m_Physics.ApplyImpulseAt(aerocenter, vDrag * timeSlice);
			}
		}
		
		// Fueslage Drag
		float sideSlipAngle = Math.Atan2(-flowVelocityLocal[0], flowVelocityLocal[2])*Math.RAD2DEG;	// [deg]
		vector vLongitudinalDrag = dynamicPressure * m_fFrontalDragArea * m_fFrontalDragCoefficient * -owner.VectorToParent(vector.Forward);
		vector vSideslipDrag = dynamicPressure * m_fSideDragArea * m_fSideDragCoefficient * sideSlipAngle * owner.VectorToParent(vector.Right);
		m_Physics.ApplyImpulseAt(coa, vLongitudinalDrag * timeSlice);
		m_Physics.ApplyImpulseAt(coa, vSideslipDrag * timeSlice);
	
		// Gear
		foreach (ADM_LandingGear gear: m_Gear)
		{
			// Gear Drag
			if (gear.GetState() > 0)
			{
				vector gearMat[4];
				gear.m_vPosition.GetWorldTransform(gearMat);
					
				float fState = gear.GetState();
				float fDrag = dynamicPressure * gear.m_fDragArea * gear.m_fDragCoefficient * fState;
				vector vDrag = fDrag * -owner.VectorToParent(vector.Forward);
				m_Physics.ApplyImpulseAt(gearMat[3], vDrag * timeSlice);
			}
		}
		
		m_SignalsManager.SetSignalValue(m_iSpeedSignal, absoluteVelocity.Length());
	}
	
	// Only calculated once since panels are static
	void CalculatePanels()
	{
		vector acTotal = vector.Zero;
		float acTotalArea = 0;
		
		for (int i = 0; i < m_Wings.Count(); i++)
		{
			m_Wings[i].CalculatePanels();
			
			acTotalArea += m_Wings[i].GetSurfaceArea();
			acTotal += m_Wings[i].GetAerodynamicCenter();
		}
		
		if (acTotalArea > 0) acTotal /= acTotalArea;
		m_vAerodynamicCenter = acTotal;
	}
	
	//------------------------------------------------------------------------------------------------
#ifdef WORKBENCH
	vector debugPoints[6];
	vector debugACLine[2];
	vector debugPointsControlSurface[6];
	void Draw(IEntity owner)
	{		
		if (!m_Wings || !owner) return;
		
		vector acTotal = vector.Zero;
		float acTotalArea = 0;
		
		// connect each section sequentially
		for (int i = 0; i < m_Wings.Count(); i++)
		{
			vector wingRoot = owner.CoordToParent(m_Wings[i].m_vRootPosition);
			vector previousSectionLE = vector.Zero;
			for (int j = 0; j < m_Wings[i].m_Sections.Count(); j++)
			{
				ADM_WingSection curSection = m_Wings[i].m_Sections[j];
				
				float surfaceArea = curSection.m_fSurfaceArea;
				vector vNormal = owner.VectorToParent(curSection.m_vNormal);
				vector vChord = owner.VectorToParent(curSection.m_vChord);
				vector vSpan = vNormal * vChord;
				
				vector acLocal = curSection.m_vAerodynamicCenter;
				vector ac = owner.CoordToParent(acLocal);
				
				float fSpan = curSection.m_Span;
				float fChord = curSection.m_Chord;
				
				float controlChordPercent = 0;
				if (curSection.m_ControlSurfaces)
				{
					for (int k = 0; k < curSection.m_ControlSurfaces.Count(); k++)
					{
						float chordPercent = curSection.m_ControlSurfaces[k].m_fChordPercent;
						if (chordPercent > controlChordPercent)
						{
							controlChordPercent = chordPercent;
						}
					}
				}
					
				// Skew points for sweep angle
				vector transformMatrix[3];
				Math3D.MatrixIdentity3(transformMatrix);
				transformMatrix[0][2] = Math.Tan(curSection.m_SweepAngle * Math.DEG2RAD);
				
				vSpan = vSpan.Multiply3(transformMatrix);
				vChord = vChord.Multiply3(transformMatrix);
				
				//--- Panel 1
				debugPoints[0] = wingRoot + previousSectionLE;
				debugPoints[1] = debugPoints[0] + vSpan * fSpan;
				debugPoints[2] = debugPoints[0] - vChord * fChord * (1-controlChordPercent);
				
				//--- Panel 2
				debugPoints[3] = debugPoints[1];
				debugPoints[4] = debugPoints[2];
				debugPoints[5] = debugPoints[0] + vSpan * fSpan - vChord * fChord * (1-controlChordPercent);
				
				previousSectionLE = debugPoints[1] - wingRoot;
				
				// AC Line
				debugACLine[0] = ac - vSpan * fSpan/2;
				debugACLine[1] = ac + vSpan * fSpan/2;
				
				acTotal += acLocal * surfaceArea;
				acTotalArea += surfaceArea;
				
				Shape.CreateTris(ARGB(100,255,0,0), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZBUFFER, debugPoints, 2);
				Shape.CreateLines(Color.CYAN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, debugACLine, 2);
				
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWNORMALSDEBUG))
					Shape.CreateArrow(ac, ac + vNormal, 0.1, Color.GREEN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER); 
				
				if (curSection.m_ControlSurfaces)
				{
					for (int k = 0; k < curSection.m_ControlSurfaces.Count(); k++)
					{
						//--- Panel 1
						debugPointsControlSurface[0] = debugPoints[4];
						debugPointsControlSurface[1] = debugPoints[5];
						debugPointsControlSurface[2] = debugPointsControlSurface[0] - vChord * fChord * curSection.m_ControlSurfaces[k].m_fChordPercent;
						
						//--- Panel 2
						debugPointsControlSurface[3] = debugPointsControlSurface[1];
						debugPointsControlSurface[4] = debugPointsControlSurface[2];
						debugPointsControlSurface[5] = debugPointsControlSurface[0] + vSpan * fSpan - vChord * fChord * curSection.m_ControlSurfaces[k].m_fChordPercent;
						
						Shape.CreateTris(ARGB(100,0,0,100), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZBUFFER, debugPointsControlSurface, 2);
					}
				}
			}
		}
		
		if (acTotalArea > 0) acTotal /= acTotalArea;
		vector coa = owner.CoordToParent(acTotal + m_vAerodynamicCenterOffset);
		Shape.CreateSphere(Color.BLUE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, coa, 0.1);	
	}
#endif
	
	//------------------------------------------------------------------------------------------------
#ifdef WORKBENCH
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		CalculatePanels();
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	bool m_ShowDbgUI = true;
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
	
#ifdef WORKBENCH
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWPANELSDEBUG))
			Draw(owner);
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWWIND))
		{
			vector center = owner.GetOrigin();
			vector wind = GetWindVector();
			Shape.CreateArrow(center, center + wind, 1, Color.YELLOW, ShapeFlags.ONCE);
		}
		
		vector vel = m_Physics.GetVelocity();	
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWATMOSPHEREDEBUG))
		{
			DbgUI.Begin(string.Format("ISA Properties: %1", owner.GetName()));
			if (!m_Physics) return;
			if (m_ShowDbgUI)
			{
				float altitude = GetAltitude();
				float density = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Density);
				float pressure = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Pressure);
				float temperature = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Temperature);
				float dynamicViscosity = ADM_InternationalStandardAtmosphere.GetDynamicViscosity(altitude);
				float reynoldsNumber = density * vel.Length() / dynamicViscosity;
				
				DbgUI.Text(string.Format("Altitude: %1 m", altitude));
				DbgUI.Text(string.Format("Density: %1 kg/m^3", density));
				DbgUI.Text(string.Format("Pressure: %1 Pa", pressure));
				DbgUI.Text(string.Format("Temperature: %1 K", temperature));
				DbgUI.Text(string.Format("Dynamic Viscosity: %1 Pa*s", dynamicViscosity));
				DbgUI.Text(string.Format("Reynolds Number: %1 m^-1", reynoldsNumber));
				DbgUI.Text("");
			}
			DbgUI.End();
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWPLANEDEBUG))
		{
			DbgUI.Begin(string.Format("ADM_FixedWing: %1", owner.GetName()));
			if (m_ShowDbgUI)
			{
				float mach = GetMachNumber();
				
				DbgUI.Text(string.Format("Velocity: %1 m/s", Math.Round(vel.Length() * 100)/100));
				DbgUI.Text(string.Format("Mach Number: %1", Math.Round(mach*100)/100));
				DbgUI.Text("");
			}
			DbgUI.End();
		}
#endif
	}
}