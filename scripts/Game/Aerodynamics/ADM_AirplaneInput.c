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
		
		input = Math.Clamp(input, -1, 1);
		
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
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void Rpc_UpdateInput(ADM_InputType input, float value, bool trim)
	{
		if (input == ADM_InputType.Aileron)
		{
			if (trim)
			{
				m_fAileronTrim = value;
			} else {
				m_fAileronInput = value;
			}
		}
		
		if (input == ADM_InputType.Elevator)
		{
			if (trim)
			{
				m_fElevatorTrim = value;
			} else {
				m_fElevatorInput = value;
			}
		}
		
		if (input == ADM_InputType.Rudder)
		{
			if (trim)
			{
				m_fRudderTrim = value;
			} else {
				m_fRudderInput = value;
			}
		}
		
		if (input == ADM_InputType.SpeedBrake)
		{
			m_fSpeedBrakeInput = value;
		}
		
		if (input == ADM_InputType.Flap)
		{
			m_fFlapInput = value;
		}
		
		if (input == ADM_InputType.Thrust)
		{
			m_fThrustInput = value;
		}
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
		{
			m_fAileronTrim += m_trimVelocity * aileron * 0.1;
			m_fAileronTrim = Math.Clamp(m_fAileronTrim, -1, 1);
			Rpc(Rpc_UpdateInput, ADM_InputType.Aileron, m_fAileronTrim, true);
		} else {
			m_fAileronInput = aileron;
			m_fAileronInput = Math.Clamp(m_fAileronInput, -1, 1);
			Rpc(Rpc_UpdateInput, ADM_InputType.Aileron, m_fAileronInput, false);
		}
	}
	
	void ElevatorInput(float elevator = 0.0, EActionTrigger reason = 0) 
	{ 
		if (!IsControlActive()) return;
		if (IsFreelookActiveForLocalPlayer()) return;
		
		if (m_TrimModifier > 0.5) {
			m_fElevatorTrim += m_trimVelocity * elevator * 0.1;
			m_fElevatorTrim = Math.Clamp(m_fElevatorTrim, -1, 1);
			Rpc(Rpc_UpdateInput, ADM_InputType.Elevator, m_fElevatorTrim, true);
		} else {
			m_fElevatorInput = elevator;
			m_fElevatorInput = Math.Clamp(m_fElevatorInput, -1, 1);
			Rpc(Rpc_UpdateInput, ADM_InputType.Elevator, m_fElevatorInput, false);
		}
	}
	
	void RudderInput(float rudder = 0.0, EActionTrigger reason = 0) 
	{
		if (!IsControlActive()) return;
		
		if (m_TrimModifier > 0.5) {
			m_fRudderTrim += m_trimVelocity * rudder * 0.1;
			m_fRudderTrim = Math.Clamp(m_fRudderTrim, -1, 1);
			Rpc(Rpc_UpdateInput, ADM_InputType.Rudder, m_fRudderTrim, true);
		} else {
			m_fRudderInput = rudder;
			m_fRudderInput = Math.Clamp(m_fRudderInput, -1, 1);
			Rpc(Rpc_UpdateInput, ADM_InputType.Rudder, m_fRudderInput, false);
		}
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
		Rpc(Rpc_UpdateInput, ADM_InputType.Thrust, m_fThrustInput, false);
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
		Rpc(Rpc_UpdateInput, ADM_InputType.SpeedBrake, m_fSpeedBrakeInput, false);
	}
	
	void FlapInput(float flap = 0.0, EActionTrigger reason = 0)
	{
		if (!IsControlActive()) return;
		
		m_fFlapInput += flap * m_fFlapVelocity;
		m_fFlapInput = Math.Clamp(m_fFlapInput, 0, 1);
		Rpc(Rpc_UpdateInput, ADM_InputType.Flap, m_fFlapInput, false);
	}
	
	void ToggleGear(float gear = 0.0, EActionTrigger reason = 0)
	{
		m_FixedWingSim.ToggleGear();
	}
	
	void WeaponRelease(float release = 0.0, EActionTrigger reason = 0)
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
		{
			return;
		}
		
		array<EntitySlotInfo> outSlotInfos = {};
		slotManager.GetSlotInfos(outSlotInfos);
		
		foreach (EntitySlotInfo slotInfo : outSlotInfos)
		{
			if (slotInfo.Type() == R3D_PylonSlotInfo)
			{
				R3D_PylonSlotInfo r3dSlotInfo = R3D_PylonSlotInfo.Cast(slotInfo);
				if (!r3dSlotInfo)
				{
					continue;
				}
				
				Print("trigger pylon");
				r3dSlotInfo.TriggerPylon();
				
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
		#ifdef WORKBENCH
		ConnectToDiagSystem(owner);
		#endif
		SetEventMask(owner, EntityEvent.FRAME);
				
		InputManager inputManager = GetGame().GetInputManager();
        inputManager.AddActionListener("R3D_AirplaneAilerons", 			EActionTrigger.VALUE, AileronInput);
        inputManager.AddActionListener("R3D_AirplaneElevators", 		EActionTrigger.VALUE, ElevatorInput);
		inputManager.AddActionListener("R3D_AirplaneTrimModifier", 		EActionTrigger.VALUE, AirplaneTrimModifier);
        inputManager.AddActionListener("R3D_AirplaneRudder", 			EActionTrigger.VALUE, RudderInput);
        inputManager.AddActionListener("R3D_AirplaneThrust", 			EActionTrigger.VALUE, ThrustInput);
        inputManager.AddActionListener("R3D_AirplaneFlaps", 			EActionTrigger.VALUE, FlapInput);
        inputManager.AddActionListener("R3D_AirplaneSpeedBrake", 		EActionTrigger.VALUE, SpeedBrakeInput);
        inputManager.AddActionListener("R3D_AirplaneToggleGear", 		EActionTrigger.DOWN,  ToggleGear);
		inputManager.AddActionListener("R3D_Freelook", 					EActionTrigger.VALUE, AirplaneFreelook);
		inputManager.AddActionListener("R3D_WeaponRelease", 			EActionTrigger.DOWN,  WeaponRelease);
		
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
	
	//------------------------------------------------------------------------------------------------
	bool m_ShowDbgUI = true;
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
	
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_R3DCORE_AIRPLANES_SHOWINPUT))
		{
			DbgUI.Begin(string.Format("ADM_AirplaneInput: %1", owner.GetName()));
			if (m_ShowDbgUI)
			{
				DbgUI.Text(string.Format("m_fAileronInput: %1", m_fAileronInput));
				DbgUI.Text(string.Format("m_fElevatorInput: %1", m_fElevatorInput));
				DbgUI.Text(string.Format("m_fRudderInput: %1", m_fRudderInput));
				DbgUI.Text(string.Format("m_fThrustInput: %1", m_fThrustInput));
				DbgUI.Text(string.Format("m_fSpeedBrakeInput: %1", m_fSpeedBrakeInput));
				DbgUI.Text(string.Format("m_fFlapInput: %1", m_fFlapInput));
				DbgUI.Text(string.Format("m_bSpeedBrakeToggle: %1", m_bSpeedBrakeToggle));
				DbgUI.Text(string.Format("m_TrimModifier: %1", m_TrimModifier));
				DbgUI.Text(string.Format("m_fAileronTrim: %1", m_fAileronTrim));
				DbgUI.Text(string.Format("m_fElevatorTrim: %1", m_fElevatorTrim));
				DbgUI.Text(string.Format("m_fRudderTrim: %1", m_fRudderTrim));
				DbgUI.Text(string.Format("m_Freelook: %1", m_Freelook));
				DbgUI.Text("");
			}
			DbgUI.End();
		}
#endif
	}
}