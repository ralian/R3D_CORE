class R3D_ToggleAircraftEngineAction : ScriptedUserAction
{
	ADM_AirplaneControllerComponent_SA m_AirplaneController;
	ADM_AirplaneInput m_Input;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_AirplaneController = ADM_AirplaneControllerComponent_SA.Cast(pOwnerEntity.FindComponent(ADM_AirplaneControllerComponent_SA));
		m_Input = ADM_AirplaneInput.Cast(pOwnerEntity.FindComponent(ADM_AirplaneInput));
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (Replication.IsServer())
		{
			m_AirplaneController.Rpc_Server_ToggleEngine();
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (m_AirplaneController.IsAirplaneEngineOn())
		{
			outName = "Stop Engine";
		}
		else
		{
			outName = "Start Engine";
		}
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return m_Input.IsControlActive();
	}
}