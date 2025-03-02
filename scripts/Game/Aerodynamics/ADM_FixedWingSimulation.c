[EntityEditorProps(category: "GameScripted/Physics", description: "Fixed Wing Simulation Component")]
class ADM_FixedWingSimulationClass: ScriptGameComponentClass
{
}

class ADM_FixedWingSimulation : ScriptGameComponent
{
	//------------------------------------------------------------------------------------------------
	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, "Contains wing sections which form the entire wing", category: "Fixed Wing Simulation")]
	ref array<ref ADM_Wing> m_Wings;
	
	[Attribute(category: "Fixed Wing Simulation", desc: "Offset for aerodynamic center. For drag calculation.")]
	protected ref PointInfo m_pAerodynamicCenterOffset;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fFrontalDragArea;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fFrontalDragCoefficient;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fSideDragArea;
	
	[Attribute(params: "0 inf", category: "Fixed Wing Simulation")]
	protected float m_fSideDragCoefficient;
	
	[Attribute(params: "0 1", defvalue: "1", category: "Fixed Wing Simulation")]
	protected float m_fWindStrength;
	
	[Attribute(uiwidget: UIWidgets.Object, category: "Fixed Wing Simulation")]
	protected ref array<ref ADM_LandingGear> m_Gear;
	
	[Attribute(category: "Fixed Wing Simulation")]
	protected string m_sHullHitzone;
	
	//------------------------------------------------------------------------------------------------
	protected ADM_FixedWingSimulationSystem m_UpdateSystem = null;
	protected ADM_AirplaneControllerComponent m_AirplaneController = null;
	protected IEntity m_Owner = null;
	protected TimeAndWeatherManagerEntity m_TimeManager = null;
	protected ChimeraWorld m_World = null;
	protected SignalsManagerComponent m_SignalsManager;
	protected RplComponent m_RplComponent;
	private SCR_VehicleDamageManagerComponent m_DamageManager = null;
	protected int m_iRPMSignal = -1;
	protected bool m_bIsDestroyed = false;
	
	protected vector m_vAerodynamicCenter = vector.Zero;
	protected vector m_vAerodynamicCenterOffset = vector.Zero;
	
	[RplProp()]
	ref array<float> m_fGearStates = {};
	
	[RplProp()]
	ref array<bool> m_bGearDeployed = {};
	
	//#ifdef WORKBENCH
	protected ref array<vector> m_vDebugForcePos = {};
	protected ref array<vector> m_vDebugForces = {};
	protected ref array<int> m_iDebugForceColor = {};
	//#endif
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToSystem()
	{
		if (!m_UpdateSystem) return;
		m_UpdateSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromSystem()
	{		
		if (!m_UpdateSystem) return;
		m_UpdateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_Owner = owner;
		m_World = owner.GetWorld();
		m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));	
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_DamageManager = SCR_VehicleDamageManagerComponent.Cast(owner.FindComponent(SCR_VehicleDamageManagerComponent));
		
		if (m_World) 
		{
			m_UpdateSystem = ADM_FixedWingSimulationSystem.Cast(m_World.FindSystem(ADM_FixedWingSimulationSystem));
			m_TimeManager = m_World.GetTimeAndWeatherManager();
		}
		
		if (m_pAerodynamicCenterOffset) 
		{
			m_pAerodynamicCenterOffset.Init(owner);
		}
		
		if (m_SignalsManager) 
		{
			m_iRPMSignal = m_SignalsManager.AddOrFindMPSignal("RPM", 1, 30, 0, SignalCompressionFunc.Range01);
		}
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnVehicleDestroyed().Insert(OnDestroyed);
		}
		
		foreach (ADM_LandingGear gear: m_Gear)
		{
			gear.m_vPosition.Init(owner);
			m_fGearStates.Insert(1);
			m_bGearDeployed.Insert(true);
		}
		
		if (owner.GetPhysics())
		{
			owner.GetPhysics().SetActive(true);
		}
		
		CalculatePanels();
		
		//#ifdef WORKBENCH
		ConnectToDiagSystem(owner);
		//#endif
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.PHYSICSACTIVE);
	}
	
	void OnDestroyed(int instigatorPlayerId)
	{
		m_bIsDestroyed = true;
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
		DisconnectFromSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		m_AirplaneController = ADM_AirplaneControllerComponent.Cast(owner.FindComponent(ADM_AirplaneControllerComponent));
		if (!m_AirplaneController)
		{
			Print("Error! ADM_FixedWingSimulation requires ADM_AirplaneControllerComponent!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromSystem();
		
		//#ifdef WORKBENCH
		DisconnectFromDiagSystem(owner);
		//#endif
		
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (activeState)
			ConnectToSystem();
		else
			DisconnectFromSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetTrim()
	{
		for (int i = 0; i < m_Wings.Count(); i++)
		{
			for (int j = 0; j < m_Wings[i].m_Sections.Count(); j++)
			{
				ADM_WingSection curSection = m_Wings[i].m_Sections[j];
				if (curSection.m_ControlSurfaces && curSection.m_ControlSurfaces.Count() > 0)
				{
					foreach(ADM_ControlSurface controlSurface: curSection.m_ControlSurfaces)
					{
						controlSurface.ResetZeroAngle();	
					}
				}
			}
		}
	}
	
	override event protected bool OnTicksOnRemoteProxy() { return true; };
	
	//------------------------------------------------------------------------------------------------
	void Simulate(float timeSlice)
	{
		if (!m_AirplaneController || !m_AirplaneController.GetAirplaneInput() || m_bIsDestroyed)
			return;
		
		// if pilot is not rpl owner, make them owner
		
		//#ifdef WORKBENCH
		m_vDebugForcePos.Clear();
		m_vDebugForces.Clear();
		m_iDebugForceColor.Clear();
		//#endif
		
		IEntity owner = m_Owner;
		vector com = GetCenterOfMass();
		vector coa = owner.CoordToParent(m_vAerodynamicCenter + m_vAerodynamicCenterOffset);
		vector wind = GetWindVector();
		
		vector absoluteVelocity = GetWorldVelocity(vector.Zero);
		
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
											
				vector sectionFlowVelocity = vector.Zero;
				if (owner.GetPhysics()) sectionFlowVelocity = owner.GetPhysics().GetVelocityAt(aerocenter) + wind;
				
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
						float deflectionAngle = controlSurface.GetAngle();
						float trimModifier = m_AirplaneController.GetAirplaneInput().GetTrimModifier();
						controlSurface.Update(timeSlice, m_AirplaneController.GetAirplaneInput().GetInput(controlSurface.m_Type), trimModifier > 0, m_AirplaneController.m_fTrimVelocityAdjustment);
						
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
				
				if (owner.GetPhysics() && absoluteVelocity.LengthSq() > 1)
				{
					owner.GetPhysics().ApplyImpulseAt(aerocenter, vLift * timeSlice);
					owner.GetPhysics().ApplyImpulseAt(aerocenter, vDrag * timeSlice);
				}
				
				//#ifdef WORKBENCH
				m_vDebugForcePos.Insert(aerocenter);
				m_vDebugForces.Insert(vLift);
				m_iDebugForceColor.Insert(Color.GREEN);
				
				m_vDebugForcePos.Insert(aerocenter);
				m_vDebugForces.Insert(vDrag);
				m_iDebugForceColor.Insert(Color.RED);
				
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWPLANEDEBUG))
				{
					Shape.CreateArrow(aerocenter, aerocenter + vSpan*3, 0.1, Color.BLACK, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
					Shape.CreateArrow(aerocenter, aerocenter - dragDir*3, 0.1, Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
					Shape.CreateArrow(aerocenter, aerocenter + liftDir*3, 0.1, Color.GREEN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
					Shape.CreateArrow(aerocenter, aerocenter - sectionFlowVelocity, 0.1, Color.BLUE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
				}
				//#endif
			}
		}
		
		// Fueslage Drag
		float sideSlipAngle = Math.Atan2(-flowVelocityLocal[0], flowVelocityLocal[2])*Math.RAD2DEG;	// [deg]
		vector vLongitudinalDrag = dynamicPressure * m_fFrontalDragArea * m_fFrontalDragCoefficient * -owner.VectorToParent(vector.Forward);
		vector vSideslipDrag = dynamicPressure * m_fSideDragArea * m_fSideDragCoefficient * sideSlipAngle * owner.VectorToParent(vector.Right);
		
		if (owner.GetPhysics() && absoluteVelocity.LengthSq() > 1)
		{
			owner.GetPhysics().ApplyImpulseAt(coa, vLongitudinalDrag * timeSlice);
			owner.GetPhysics().ApplyImpulseAt(coa, vSideslipDrag * timeSlice);
		}
		
		//#ifdef WORKBENCH
		m_vDebugForcePos.Insert(coa);
		m_vDebugForces.Insert(vLongitudinalDrag);
		m_iDebugForceColor.Insert(Color.ORANGE);
		
		m_vDebugForcePos.Insert(coa);
		m_vDebugForces.Insert(vSideslipDrag);
		m_iDebugForceColor.Insert(Color.ORANGE);
		//#endif
		
		int i = 0;
		foreach (ADM_LandingGear gear: m_Gear)
		{
			float fState = m_fGearStates[i];
			if (fState > 0)
			{
				vector gearMat[4];
				gear.m_vPosition.GetWorldTransform(gearMat);
				
				float fDrag = dynamicPressure * gear.m_fDragArea * gear.m_fDragCoefficient * fState;
				vector vDrag = fDrag * -owner.VectorToParent(vector.Forward);
				
				if (owner.GetPhysics() && absoluteVelocity.LengthSq() > 1)
				{
					owner.GetPhysics().ApplyImpulseAt(gearMat[3], vDrag * timeSlice);
				}
				
				//#ifdef WORKBENCH
				m_vDebugForcePos.Insert(gearMat[3]);
				m_vDebugForces.Insert(vDrag);
				m_iDebugForceColor.Insert(Color.RED);
				//#endif
			}
			
			if (m_SignalsManager)
			{
				if (gear.m_iSignalIndex == -1)
				{
					gear.m_iSignalIndex = m_SignalsManager.AddOrFindMPSignal(gear.m_sSignal, 0.05, 30, 0, SignalCompressionFunc.RotDEG);
				}
				
				float signal = (1 - m_fGearStates[i]) * gear.m_fRotationAngle;
				m_SignalsManager.SetSignalValue(gear.m_iSignalIndex, signal);
			}
			
			if (m_bGearDeployed[i] && m_fGearStates[i] < 1 && !(gear.m_fRotationRate <= 0 || gear.m_fRotationAngle == 0))
			{
				m_fGearStates[i] = Math.Clamp(m_fGearStates[i] + gear.m_fRotationRate/gear.m_fRotationAngle * timeSlice, 0, 1);
				Replication.BumpMe();
			}
			
			if (!m_bGearDeployed[i] && m_fGearStates[i] > 0 && !(gear.m_fRotationRate <= 0 || gear.m_fRotationAngle == 0))
			{
				m_fGearStates[i] = Math.Clamp(m_fGearStates[i] - gear.m_fRotationRate/gear.m_fRotationAngle * timeSlice, 0, 1);
				Replication.BumpMe();
			}
			
			i++;
		}
		
		foreach (ADM_EngineComponent engine : m_AirplaneController.GetEngines())
		{
			engine.Simulate(owner, timeSlice);
		}
		
		if (m_SignalsManager)
		{
			m_SignalsManager.SetSignalValue(m_iRPMSignal, m_AirplaneController.GetAirplaneInput().GetInput(ADM_InputType.Thrust));
		}		
	}
	
	//------------------------------------------------------------------------------------------------
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
	vector GetWindVector()
	{		
		if (!m_TimeManager) 
			return vector.Zero;
		
		vector angles = vector.Zero;
		float speed = m_TimeManager.GetWindSpeed();
		angles[0] = m_TimeManager.GetWindDirection();
		
		vector mat[3];
		Math3D.AnglesToMatrix(angles, mat);
		
		return speed * mat[2] * m_fWindStrength;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAltitude()
	{
		return m_Owner.GetOrigin()[1];
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetCenterOfMass()
	{
		if (!m_Owner || !m_Owner.GetPhysics())
			return vector.Zero;
		
		return m_Owner.CoordToParent(m_Owner.GetPhysics().GetCenterOfMass()); // world coordinates
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetWorldVelocity(vector pos = vector.Zero)
	{		
		if (!m_Owner || !m_Owner.GetPhysics())
			return vector.Zero;
		
		return m_Owner.GetPhysics().GetVelocityAt(pos); 
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetTrueAirVelocity(vector pos = vector.Zero)
	{
		return GetWorldVelocity(pos) + GetWindVector();
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref ADM_LandingGear> GetGear()
	{
		return m_Gear;
	}
	
	//------------------------------------------------------------------------------------------------
//#ifdef WORKBENCH
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
		if (m_Owner && m_Owner.GetPhysics()) Shape.CreateSphere(Color.YELLOW, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, GetCenterOfMass(), 0.1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DebugMenu(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWPANELSDEBUG))
			Draw(owner);
		
		vector com = GetCenterOfMass();
		vector coa = owner.CoordToParent(m_vAerodynamicCenter + m_vAerodynamicCenterOffset);
		vector flowVelocity = GetTrueAirVelocity(com);
		vector wind = GetWindVector();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWWIND))
		{
			Shape.CreateArrow(com, com + wind, 1, Color.YELLOW, ShapeFlags.ONCE);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWATMOSPHEREDEBUG))
		{
			DbgUI.Begin(string.Format("ISA Properties: %1", owner.GetName()));
			float altitude = GetAltitude();
			float density = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Density);
			float pressure = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Pressure);
			float temperature = ADM_InternationalStandardAtmosphere.GetValue(altitude, ADM_ISAProperties.Temperature);
			float dynamicViscosity = ADM_InternationalStandardAtmosphere.GetDynamicViscosity(altitude);
			float reynoldsNumber = density * flowVelocity.Length() / dynamicViscosity;
			
			DbgUI.Text(string.Format("Altitude: %1 m", altitude));
			DbgUI.Text(string.Format("Density: %1 kg/m^3", density));
			DbgUI.Text(string.Format("Pressure: %1 Pa", pressure));
			DbgUI.Text(string.Format("Temperature: %1 K", temperature));
			DbgUI.Text(string.Format("Dynamic Viscosity: %1 Pa*s", dynamicViscosity));
			DbgUI.Text(string.Format("Reynolds Number: %1 m^-1", reynoldsNumber));
			DbgUI.Text("");
			DbgUI.End();
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWPLANEDEBUG))
		{
			DbgUI.Begin(string.Format("ADM_FixedWing: %1", owner.GetName()));
			float turningRadius = 0;
			
			DbgUI.Text(string.Format("World Velocity COM: %1 m/s", Math.Round(GetWorldVelocity(com).Length() * 100)/100));
			DbgUI.Text(string.Format("True Air Speed COM: %1 m/s", Math.Round(flowVelocity.Length() * 100)/100));
			DbgUI.Text(string.Format("Turning Radius: %1 ft", Math.Round(turningRadius*100)/100));
			DbgUI.Text("");
			DbgUI.Text("Gear:");
			for (int i=0; i < m_Gear.Count(); i++)
			{
				DbgUI.Text(string.Format(" - %1: %2", i, m_fGearStates[i]));
			}
			DbgUI.End();
			
			//#ifdef WORKBENCH
			float maxForce = 1;
			for (int i = 0; i < m_vDebugForces.Count(); i++)
			{
				if (m_vDebugForces[i].Length() > maxForce) 
					maxForce = m_vDebugForces[i].Length();
			}
			
			for (int i = 0; i < m_vDebugForces.Count(); i++)
			{
				Shape.CreateArrow(m_vDebugForcePos[i], m_vDebugForcePos[i] + 5*m_vDebugForces[i]/maxForce, 0.1, m_iDebugForceColor[i], ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
			}
			//#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event void _WB_OnParentChange(IEntity owner, IEntitySource src, IEntitySource prevParentSrc)
	{
		CalculatePanels();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DebugMenu(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
		DebugMenu(owner, timeSlice);
	}
//#endif
}