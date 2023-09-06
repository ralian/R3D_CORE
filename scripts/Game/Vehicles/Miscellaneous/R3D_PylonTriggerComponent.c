class R3D_PylonTriggerComponentClass : ScriptComponentClass {}

class R3D_PylonTriggerComponent : ScriptComponent 
{
	// Mod me for custom behavior on drop. (like triggering a missile)
	bool Trigger(IEntity owner, R3D_PylonComponent fromPylon)
	{
		if (fromPylon.CanUnloadItem())
		{
			IEntity attachedEntity = fromPylon.item;
			fromPylon.UnloadItem();
			
			if (attachedEntity)
			{
				BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(attachedEntity.FindComponent(BaseTriggerComponent));
				if (triggerComp) triggerComp.SetLive();
				
				R3D_ExplosionTriggerComponent redTriggerComp = R3D_ExplosionTriggerComponent.Cast(attachedEntity.FindComponent(R3D_ExplosionTriggerComponent));
				if (redTriggerComp) 
				{ 
					//redTriggerComp.IgnoreEntityOnCollision(fromPylon.m_owner.GetRootParent());
					redTriggerComp.TriggerArmExplosive();
				}
			}
			
			return true;
		}
		
		return false;
	}
}