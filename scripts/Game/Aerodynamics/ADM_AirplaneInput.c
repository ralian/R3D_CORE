enum ADM_InputType
{
	/*
		0 m_fAileronInput
		1 m_fElevatorInput
		2 m_fRudderInput
		3 m_fThrustInput
		4 m_fSpeedBrakeInput
	*/
	Aileron = 0,
	Elevator = 1,
	Rudder = 2,
	Thrust = 3,
	SpeedBrake = 4,
	Flap = 5
}

[EntityEditorProps(category: "GameScripted/Aerodynamics", description: "")]
class ADM_AirplaneInputClass: ScriptComponentClass
{
};

class ADM_AirplaneInput : ScriptComponent
{		
	protected float m_fAileronInput = 0.0;
	protected float m_fElevatorInput = 0.0;
	protected float m_fRudderInput = 0.0;
	protected float m_fThrustInput = 0.0;
	protected float m_fSpeedBrakeInput = 0.0;
	protected float m_fFlapInput = 0.0;
	protected bool m_bSpeedBrakeToggle = false;
	
	protected float m_fSteeringInput = 0.0;
	protected float m_fWheelBrakeInput = 0.0;
	protected float m_fHandBrakeInput = 0.0;
	protected bool m_bParkingBrakeInput = true;
	protected float m_fTrimModifierInput = 0.0;
	protected float m_fFOVZoom = 0.0;
	
	protected float m_fSteeringAngle = 0.0;
	
	protected ADM_AirplaneControllerComponent m_AirplaneController;
	protected RplComponent m_RplComponent;
	
	float GetInput(ADM_InputType type)
	{
		float input = 0;
		switch (type) {
			case ADM_InputType.Flap:
			{
				input += m_fFlapInput;
				break;
			}
			case ADM_InputType.Elevator:
			{
				input += m_fElevatorInput;
				break;
			}
			case ADM_InputType.Aileron:
			{
				input += m_fAileronInput;
				break;
			}
			case ADM_InputType.Rudder:
			{
				input += m_fRudderInput;
				break;
			}
			case ADM_InputType.SpeedBrake:
			{
				input += m_fSpeedBrakeInput;
				break;
			}
			case ADM_InputType.Thrust:
			{
				input += m_fThrustInput;
				break;
			}
		}
		
		input = Math.Clamp(input, -1, 1);
		
		return input;
	}
	
	void AileronInput(float aileron = 0.0, EActionTrigger reason = 0) 
	{
		if (!IsControlActive()) return;
		
		m_fAileronInput = aileron;
		m_fAileronInput = Math.Clamp(m_fAileronInput, -1, 1);
	}
	
	void ElevatorInput(float elevator = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		
		m_fElevatorInput = elevator;
		m_fElevatorInput = Math.Clamp(m_fElevatorInput, -1, 1);
	}
	
	void RudderInput(float rudder = 0.0, EActionTrigger reason = 0) 
	{
		if (!IsControlActive()) return;
		
		m_fRudderInput = rudder;
		m_fRudderInput = Math.Clamp(m_fRudderInput, -1, 1);
	}
	
	void ThrustInput(float thrust = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		
		m_fThrustInput += thrust * m_AirplaneController.m_fThrustVelocity; 
		m_fThrustInput = Math.Clamp(m_fThrustInput, 0, 1);
	}
	
	void SpeedBrakeInput(float airBrake = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		
		m_fSpeedBrakeInput = airBrake; 
		m_fSpeedBrakeInput = Math.Clamp(m_fSpeedBrakeInput, -1, 1);
		
		if (airBrake > 0) 
		{
			m_bSpeedBrakeToggle = true;
		} else {
			m_bSpeedBrakeToggle = false;
		}
	}
	
	void FlapInput(float flap = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		
		m_fFlapInput = flap;
		m_fFlapInput = Math.Clamp(m_fFlapInput, -1, 1);
	}
	
	void ToggleGear(float gear = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_AirplaneController.ToggleGear();
	}
	
	void TrimReset(float trimReset = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_AirplaneController.ResetTrim();
	}
	
	void Steering(float steering = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_fSteeringInput = steering;
	}
	
	void WheelBrake(float wheelBrake = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_fWheelBrakeInput = wheelBrake;
	}
	
	void HandBrake(float handBrake = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_fHandBrakeInput = handBrake;
	}
	
	void ParkingBrake(float parkingBrake = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_bParkingBrakeInput = !m_bParkingBrakeInput;
	}
	
	void TrimModifier(float trimModifier = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		m_fTrimModifierInput = trimModifier;
	}
	
	void AdjustFOV(float input = 0.0, EActionTrigger reason = 0) 
	{ 		
		m_fFOVZoom += 0.05*input; 
		m_fFOVZoom = Math.Clamp(m_fFOVZoom, 0, 1);
		
		if (input != 0)
			m_AirplaneController.UpdateFOV(m_fFOVZoom);
	}
	
	float GetTrimModifier()
	{
		return m_fTrimModifierInput;
	}
	
	float GetFOVZoom()
	{
		return m_fFOVZoom;
	}
	
	bool IsControlActive()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return false;
		
		PlayerController playerController = game.GetPlayerController();
		if (!playerController)
			return false;
		
		IEntity player = playerController.GetControlledEntity();
		if (!player)
			return false;
		
		IEntity parent = player.GetParent();
		return (parent && parent == GetOwner());
	}
	
	protected float m_fMaxCenterX;
	protected float m_fMaxForwardX;
	protected float m_fMaxBackwardX;
	override void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		ConnectToDiagSystem(owner);
		#endif
				
		m_AirplaneController = ADM_AirplaneControllerComponent.Cast(owner.FindComponent(ADM_AirplaneControllerComponent));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		InputManager inputManager = GetGame().GetInputManager();
        inputManager.AddActionListener("R3D_AirplaneAilerons", 			EActionTrigger.VALUE,  AileronInput);
        inputManager.AddActionListener("R3D_AirplaneElevators", 		EActionTrigger.VALUE,  ElevatorInput);
        inputManager.AddActionListener("R3D_AirplaneRudder", 			EActionTrigger.VALUE,  RudderInput);
        inputManager.AddActionListener("R3D_AirplaneThrust", 			EActionTrigger.VALUE,  ThrustInput);
        inputManager.AddActionListener("R3D_AirplaneFlaps", 			EActionTrigger.VALUE,  FlapInput);
        inputManager.AddActionListener("R3D_AirplaneSpeedBrake", 		EActionTrigger.VALUE,  SpeedBrakeInput);
        inputManager.AddActionListener("R3D_AirplaneToggleGear", 		EActionTrigger.DOWN,   ToggleGear);
		inputManager.AddActionListener("R3D_AirplaneTrimModifier",		EActionTrigger.VALUE,  TrimModifier);
		inputManager.AddActionListener("R3D_AirplaneTrimReset",			EActionTrigger.DOWN,   TrimReset);
		inputManager.AddActionListener("R3D_AirplaneSteering",			EActionTrigger.VALUE,  Steering);
		inputManager.AddActionListener("R3D_AirplaneWheelBrake",		EActionTrigger.VALUE,  WheelBrake);
		inputManager.AddActionListener("R3D_AirplaneHandBrake",			EActionTrigger.VALUE,  HandBrake);
		inputManager.AddActionListener("R3D_AirplaneParkingBrake",		EActionTrigger.DOWN,   ParkingBrake);
		inputManager.AddActionListener("R3D_AirplaneAdjustFOV",			EActionTrigger.VALUE,  AdjustFOV);
		
		for (int i = 0; i < m_AirplaneController.m_fSteeringForwardStrength.Count(); i++)
		{
			vector strength = m_AirplaneController.m_fSteeringForwardStrength[i];
			if (strength[0] > m_fMaxForwardX)
				m_fMaxForwardX = strength[0];
		}
		for (int i = 0; i < m_AirplaneController.m_fSteeringBackwardStrength.Count(); i++)
		{
			vector strength = m_AirplaneController.m_fSteeringBackwardStrength[i];
			if (strength[0] > m_fMaxBackwardX)
				m_fMaxBackwardX = strength[0];
		}
		for (int i = 0; i < m_AirplaneController.m_fSteeringCenterStrength.Count(); i++)
		{
			vector strength = m_AirplaneController.m_fSteeringCenterStrength[i];
			if (strength[0] > m_fMaxCenterX)
				m_fMaxCenterX = strength[0];
		}
		
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	void UpdateSteering(float timeSlice)
	{
		float speed = m_AirplaneController.GetSimulation().GetSpeedKmh(); // [km/h]
		//float dist = Math.Clamp(speed/m_fMaxCenterX, 0, 1);
		float steeringStrength = Math3D.Curve(ECurveType.CurveProperty2D, speed, m_AirplaneController.m_fSteeringCenterStrength)[1]; // [deg/s]
		
		// if going away from center use m_fSteeringForwardStrength
		if ( (m_fSteeringInput > 0 && m_fSteeringAngle >= 0) || (m_fSteeringInput < 0 && m_fSteeringAngle <= 0) )
		{
			//dist = Math.Clamp(speed/m_fMaxForwardX, 0, 1);
			steeringStrength = Math3D.Curve(ECurveType.CurveProperty2D, speed, m_AirplaneController.m_fSteeringForwardStrength)[1]; 
		}
		
		// if going toward center use m_fSteeringBackwardStrength
		if ( (m_fSteeringInput > 0 && m_fSteeringAngle < 0) || (m_fSteeringInput < 0 && m_fSteeringAngle > 0) )
		{
			//dist = Math.Clamp(speed/m_fMaxBackwardX, 0, 1);
			steeringStrength = Math3D.Curve(ECurveType.CurveProperty2D, speed, m_AirplaneController.m_fSteeringBackwardStrength)[1]; 
		}
		
		float targetAngle = m_AirplaneController.m_fMaxSteerAngle;
		if (m_fSteeringInput < 0)
			targetAngle *= -1;
		
		if (m_fSteeringInput == 0 && m_AirplaneController.m_bAutoZeroSteerAngle)
			targetAngle = 0;
		
		if (m_fSteeringInput == 0 && !m_AirplaneController.m_bAutoZeroSteerAngle)
			targetAngle = m_fSteeringAngle;
		
		targetAngle *= Math.AbsFloat(m_fSteeringInput);
		
		float delta = steeringStrength*timeSlice;
		float error = targetAngle - m_fSteeringAngle;
		if (delta > Math.AbsFloat(error))
			delta = Math.AbsFloat(error);
		
		if (error < 0)
			delta *= -1;
		
		m_fSteeringAngle += delta;
		m_fSteeringAngle = Math.Clamp(m_fSteeringAngle, -m_AirplaneController.m_fMaxSteerAngle, m_AirplaneController.m_fMaxSteerAngle);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!m_AirplaneController) return;
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager && IsControlActive())
		{
			// Attempt to ignore car inputs
			inputManager.ResetContext("CarContext");
			inputManager.ActivateContext("R3D_AirplaneContext");
			
			// Apply custom car inputs (steering, brakes)
			UpdateSteering(timeSlice);
			inputManager.SetActionValue("CarSteering", m_fSteeringAngle/m_AirplaneController.m_fMaxSteerAngle);
			inputManager.SetActionValue("CarBrake", m_fWheelBrakeInput);
			inputManager.SetActionValue("CarHandBrake", m_fHandBrakeInput);
			m_AirplaneController.SetPersistentHandBrake(m_bParkingBrakeInput);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
	
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWINPUT))
		{
			DbgUI.Begin(string.Format("ADM_AirplaneInput: %1", owner.GetName()));
			DbgUI.Text(string.Format("m_fAileronInput: %1", m_fAileronInput));
			DbgUI.Text(string.Format("m_fElevatorInput: %1", m_fElevatorInput));
			DbgUI.Text(string.Format("m_fRudderInput: %1", m_fRudderInput));
			DbgUI.Text(string.Format("m_fThrustInput: %1", m_fThrustInput));
			DbgUI.Text(string.Format("m_fSpeedBrakeInput: %1", m_fSpeedBrakeInput));
			DbgUI.Text(string.Format("m_fFlapInput: %1", m_fFlapInput));
			DbgUI.Text(string.Format("m_bSpeedBrakeToggle: %1", m_bSpeedBrakeToggle));
			DbgUI.Text(string.Format("m_fSteeringInput: %1", m_fSteeringInput));
			DbgUI.Text(string.Format("m_fWheelBrakeInput: %1", m_fWheelBrakeInput));
			DbgUI.Text(string.Format("m_fHandBrakeInput: %1", m_fHandBrakeInput));
			DbgUI.Text(string.Format("m_bParkingBrakeInput: %1", m_bParkingBrakeInput));
			DbgUI.Text(string.Format("m_fTrimModifierInput: %1", m_fTrimModifierInput));
			DbgUI.Text(string.Format("m_fFOVZoom: %1", m_fFOVZoom));
			DbgUI.Text("");
			DbgUI.End();
		}
#endif
	}
}