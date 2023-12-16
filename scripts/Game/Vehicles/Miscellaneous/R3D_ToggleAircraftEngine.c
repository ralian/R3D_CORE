class R3D_ToggleAircraftEngineAction : ScriptedUserAction
{
	ADM_FixedWingSimulation m_FixedWing;
	ADM_AirplaneInput m_Input;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_FixedWing = ADM_FixedWingSimulation.Cast(pOwnerEntity.FindComponent(ADM_FixedWingSimulation));
		m_Input = ADM_AirplaneInput.Cast(pOwnerEntity.FindComponent(ADM_AirplaneInput));
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (Replication.IsServer())
		{
			m_FixedWing.Rpc_Server_ToggleEngine();
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (m_FixedWing.IsEngineOn())
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