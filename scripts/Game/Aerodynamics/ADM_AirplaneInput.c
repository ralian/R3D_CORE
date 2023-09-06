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

//! A brief explanation of what this component does.
//! The explanation can be spread across multiple lines.
//! This should help with quickly understanding the script's purpose.
class ADM_AirplaneInput : ScriptComponent
{	
	[Attribute(category: "Fixed Wing Simulation", defvalue: "0.01")]
	protected float m_fThrustVelocity;
	
	[Attribute(category: "Fixed Wing Simulation", defvalue: "0.005")]
	protected float m_fSpeedBrakeVelocity;
	
	[Attribute(category: "Fixed Wing Simulation", defvalue: "0.005")]
	protected float m_fFlapVelocity;
	
	[Attribute(defvalue: "0.02", category: "Fixed Wing Simulation")]
	protected float m_trimVelocity;
	
	protected float m_fAileronInput = 0.0;
	protected float m_fElevatorInput = 0.0;
	protected float m_fRudderInput = 0.0;
	protected float m_fThrustInput = 0.0;
	protected float m_fSpeedBrakeInput = 0.0;
	protected float m_fFlapInput = 0.0;
	protected bool m_bSpeedBrakeToggle = false;
	
	protected float m_TrimModifier = 0.0;
	
	protected float m_fAileronTrim = 0.0;
	protected float m_fElevatorTrim = 0.0;
	protected float m_fRudderTrim = 0.0;
	
	protected float m_Freelook = 0.0;
	
	protected ADM_FixedWingSimulation m_FixedWingSim;
	
	float GetInput(ADM_InputType type)
	{
		float input = GetTrim(type);
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
		
		Math.Clamp(input, -1, 1);
		
		return input;
	}
	
	float GetTrim(ADM_InputType type)
	{
		switch (type) {
			case ADM_InputType.Elevator:
				return m_fElevatorTrim;
			case ADM_InputType.Aileron:
				return m_fAileronTrim;
			case ADM_InputType.Rudder:
				return m_fRudderTrim;
		}
		
		return 0.0;
	}
	
	void AirplaneTrimModifier(float trimModifier = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		
		m_TrimModifier = trimModifier;
	}
	
	void AileronInput(float aileron = 0.0, EActionTrigger reason = 0) 
	{
		if (!IsControlActive()) return;
		if (IsFreelookActiveForLocalPlayer()) return;
		
		if (m_TrimModifier > 0.5)
			m_fAileronTrim += m_trimVelocity * aileron * 0.1;
		else
			m_fAileronInput = aileron;
	}
	
	void ElevatorInput(float elevator = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		if (IsFreelookActiveForLocalPlayer()) return;
		
		if (m_TrimModifier > 0.5)
			m_fElevatorTrim += m_trimVelocity * elevator * 0.1;
		else
			m_fElevatorInput = elevator;
	}
	
	void RudderInput(float rudder = 0.0, EActionTrigger reason = 0) 
	{
		if (!IsControlActive()) return;
		
		if (m_TrimModifier > 0.5)
			m_fRudderTrim += m_trimVelocity * rudder * 0.1;
		else
			m_fRudderInput = rudder;
	}
	
	void ThrustInput(float thrust = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		
		if (m_TrimModifier > 0.5)
		{
			InputManager im = g_Game.GetInputManager();
			if (im && !im.IsUsingMouseAndKeyboard())
			{
				// Hacky way to use the trim modifier for flaps on controller
				m_fFlapInput += -thrust * m_fFlapVelocity;
				m_fFlapInput = Math.Clamp(m_fFlapInput, 0, 1);
				return;
			}
		}
		
		m_fThrustInput += thrust * m_fThrustVelocity; 
		m_fThrustInput = Math.Clamp(m_fThrustInput, 0, 1);
	}
	
	void SpeedBrakeInput(float airBrake = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		
		m_fSpeedBrakeInput += airBrake * m_fSpeedBrakeVelocity; 
		m_fSpeedBrakeInput = Math.Clamp(m_fSpeedBrakeInput, 0, 1);
		
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
		
		m_fFlapInput += flap * m_fFlapVelocity;
		m_fFlapInput = Math.Clamp(m_fFlapInput, 0, 1);
	}
	
	void ToggleGear(float gear = 0.0, EActionTrigger reason = 0)
	{
		m_FixedWingSim.ToggleGear();
	}
	
	void WeaponRelease(float release = 0.0, EActionTrigger reason = 0)
	{
		array<Managed> components = {};
		GetOwner().FindComponents(R3D_PylonComponent, components);
		
		foreach (Managed cmp : components)
		{
			R3D_PylonComponent pyCmp = R3D_PylonComponent.Cast(cmp);
			
			if (pyCmp)
			{
				if (pyCmp.TriggerPylon())
					return;
			}
		}
	}
	
	void AirplaneFreelook(float freelook = 0.0, EActionTrigger reason = 0)
	{
		m_Freelook = freelook;
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
	
	bool IsFreelookActiveForLocalPlayer()
	{
		if (!g_Game) return false;
		
		InputManager im = g_Game.GetInputManager();
		if (!im) return false;
		if (im.IsUsingMouseAndKeyboard()) m_Freelook = false; // M&K always freelooks independently, no need to freeze controls
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc) return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetLocalControlledEntity());
		if (!player)
			return false;
		
		CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent)).SetDisableViewControls(!(m_Freelook || im.IsUsingMouseAndKeyboard()));
		
		return m_Freelook;
	}
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
		
		InputManager inputManager = GetGame().GetInputManager();
        inputManager.AddActionListener("R3D_AirplaneAilerons", 		EActionTrigger.VALUE, AileronInput);
        inputManager.AddActionListener("R3D_AirplaneElevators", 	EActionTrigger.VALUE, ElevatorInput);
		inputManager.AddActionListener("R3D_AirplaneTrimModifier", 	EActionTrigger.VALUE, AirplaneTrimModifier);
        inputManager.AddActionListener("R3D_AirplaneRudder", 		EActionTrigger.VALUE, RudderInput);
        inputManager.AddActionListener("R3D_AirplaneThrust", 		EActionTrigger.VALUE, ThrustInput);
        inputManager.AddActionListener("R3D_AirplaneFlaps", 		EActionTrigger.VALUE, FlapInput);
        inputManager.AddActionListener("R3D_AirplaneSpeedBrake", 	EActionTrigger.VALUE, SpeedBrakeInput);
        inputManager.AddActionListener("R3D_AirplaneToggleGear", 	EActionTrigger.DOWN, ToggleGear);
		inputManager.AddActionListener("R3D_Freelook", 				EActionTrigger.VALUE, AirplaneFreelook);
		inputManager.AddActionListener("R3D_WeaponRelease", 		EActionTrigger.DOWN, WeaponRelease);
		
		m_FixedWingSim = ADM_FixedWingSimulation.Cast(owner.FindComponent(ADM_FixedWingSimulation));
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!m_FixedWingSim) return;
		if (IsControlActive())
		{
			GetGame().GetInputManager().ActivateContext("R3D_AirplaneContext");
		}
			
		if (m_fSpeedBrakeInput > 0 && !m_bSpeedBrakeToggle)
		{
			m_fSpeedBrakeInput -= m_fSpeedBrakeVelocity;
			m_fSpeedBrakeInput = Math.Clamp(m_fSpeedBrakeInput, 0, 1);
		}
	}
}