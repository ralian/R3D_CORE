/*class ADM_CharacterCommandAircraftInputST
{
	void ADM_CharacterCommandAircraftInputST()
	{
	}	
	
	bool Bind(CharacterAnimationComponent animationComponent)
	{
		m_CommandAircraftInput = animationComponent.BindCommand("CMD_AircraftInput");
		
		m_VarAileronInput	= animationComponent.BindVariableFloat("AircraftAileronInput");
		m_VarElevatorInput	= animationComponent.BindVariableFloat("AircraftElevatorInput");
		
		m_bIsBound = m_CommandAircraftInput != -1 && m_VarAileronInput != -1 && m_VarElevatorInput != -1;
		return m_bIsBound;
	}
	
	bool IsBound()
	{
		return m_bIsBound;
	}

	TAnimGraphCommand 		m_CommandAircraftInput = -1;
	TAnimGraphVariable 		m_VarAileronInput;				//! float variable - angle deg
	TAnimGraphVariable 		m_VarElevatorInput;				//! float variable - angle deg
	
	protected bool m_bIsBound = false;
}

class ADM_CharacterCommandAircraftInput : ScriptedCommand
{
	//! constructor
	void ADM_CharacterCommandAircraftInput(BaseAnimPhysComponent pAnimPhysComponent, ADM_CharacterCommandAircraftInputST pTable, ChimeraCharacter pCharacter, CharacterControllerComponent pController)
	{
		m_pCharacter 			= pCharacter;
		m_AnimationComponent 	= CharacterAnimationComponent.Cast(pAnimPhysComponent);		
		m_Input					= pController.GetInputContext();
		m_Table 				= pTable;
		
		//Print("ADM_CharacterCommandAircraftInput");
	}
	
	void FindAircraftInput()
	{
		PlayerController playerController = PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController) return;

		if (!playerController.GetControlledEntity()) return;
		m_AircraftInput = ADM_AirplaneInput.Cast(playerController.GetControlledEntity().FindComponent(ADM_AirplaneInput));
		
		Print(playerController.GetControlledEntity());
		Print(m_AircraftInput);
	}
	
	//! 
	override void OnActivate()
	{
		super.OnActivate();
		
		//Print("ADM_CharacterCommandAircraftInput::OnActivate");
		FindAircraftInput();
	}

	override void OnDeactivate()
	{
		super.OnDeactivate();
		//Print("ADM_CharacterCommandAircraftInput::OnDeactivate");
	}

	// called to set values to animation graph processing 
	override void PreAnimUpdate(float pDt)
	{		
		super.PreAnimUpdate(pDt);
		
		Print("ADM_CharacterCommandAircraftInput::PreAnimUpdate: " + pDt.ToString());
		
		if (m_AircraftInput)
		{
			PreAnim_SetFloat(m_Table.m_VarAileronInput, m_AircraftInput.GetInput(ADM_InputType.Aileron));
			PreAnim_SetFloat(m_Table.m_VarElevatorInput, m_AircraftInput.GetInput(ADM_InputType.Elevator));
		} else {
			FindAircraftInput();
		}
		
		//! update time
		//m_fTime += pDt;
	}

	ChimeraCharacter					m_pCharacter;
	CharacterInputContext 				m_Input;
	CharacterAnimationComponent			m_AnimationComponent;
	ADM_AirplaneInput 					m_AircraftInput;
	
	ADM_CharacterCommandAircraftInputST	m_Table;
}*/