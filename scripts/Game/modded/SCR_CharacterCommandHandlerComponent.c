/*modded class SCR_CharacterCommandHandlerComponent
{	
	void StartCommandAircraftInput()
	{
		if (IsAircraftInputActive())
			return;
		
		SCR_CharacterControllerComponent scrCharCtrl = SCR_CharacterControllerComponent.Cast(GetControllerComponent());
		if (!scrCharCtrl)
			return;

		m_CmdAircraftInput = new ADM_CharacterCommandAircraftInput(m_CharacterAnimComp, m_AircraftInputTable, m_OwnerEntity, scrCharCtrl);
		m_CharacterAnimComp.SetCurrentCommand(m_CmdAircraftInput);
	}

	protected ADM_CharacterCommandAircraftInput GetAircraftInputCommand()
	{
		AnimPhysCommandScripted currentCmdScripted = m_CharacterAnimComp.GetCommandScripted();
		if (!currentCmdScripted)
			return null;
		
		return ADM_CharacterCommandAircraftInput.Cast(currentCmdScripted);
	}

	bool IsAircraftInputActive()
	{
		return GetAircraftInputCommand() != null;
	}
	
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		//Print("SCR_CharacterCommandHandlerComponent init");
		m_AircraftInputTable = new ADM_CharacterCommandAircraftInputST();
		
		if (!m_AircraftInputTable.Bind(m_CharacterAnimComp))
		{
			Print("Failed to bind scripted aicraft input table (see class ADM_CharacterCommandAircraftInputST). This can be caused by missing animation commands, tags, or events in the animation graph.");
		}
		
		//Print(m_AircraftInputTable.IsBound());
		//Print(m_CmdAircraftInput);
	}
		
	protected ref ADM_CharacterCommandAircraftInputST	m_AircraftInputTable;
	protected ref ADM_CharacterCommandAircraftInput 	m_CmdAircraftInput;
}*/