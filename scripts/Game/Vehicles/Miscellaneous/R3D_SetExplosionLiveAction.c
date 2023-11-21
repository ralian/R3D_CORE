class R3D_SetExplosionLiveAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(pOwnerEntity.FindComponent(BaseTriggerComponent));
		if (triggerComp) 
		{
			triggerComp.SetLive();
		}
	}
	
	override bool CanBePerformedScript(IEntity user) 
	{ 
		return true; 
	}
	
	override bool GetActionNameScript(out string outName) 
	{
		outName = "Set Explosive Live";
		return true;
	}	
}