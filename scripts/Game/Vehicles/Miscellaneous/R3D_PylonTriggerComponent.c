class R3D_PylonTriggerComponentClass : ScriptComponentClass {}

class R3D_PylonTriggerComponent : ScriptComponent 
{
	// Mod me for custom behavior on drop. (like triggering a missile)
	bool Trigger(R3D_PylonSlotInfo fromPylon)
	{
		if (fromPylon.CanUnload())
		{
			IEntity attachedEntity = fromPylon.GetAttachedEntity();
			if (!attachedEntity)
				return false;
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (!playerController)
				return false;
			
			IEntity player = playerController.GetMainEntity();	
			if (!player)
				return false;
			
			ActionsPerformerComponent actionPerformer = ActionsPerformerComponent.Cast(player.FindComponent(ActionsPerformerComponent));
			if (!actionPerformer)
				return false;
			
			ActionsManagerComponent actionsManagerPylon = ActionsManagerComponent.Cast(fromPylon.GetOwner().FindComponent(ActionsManagerComponent));
			if (!actionsManagerPylon)
				return false;
			
			R3D_MngPylonAction dropAction;
			array<BaseUserAction> outActions = {};
			actionsManagerPylon.GetActionsList(outActions);
			foreach (BaseUserAction action : outActions)
			{
				R3D_MngPylonAction pylonAction = R3D_MngPylonAction.Cast(action);
				if (pylonAction && pylonAction.m_sPylonSlot == fromPylon.GetSourceName())
				{
					dropAction = pylonAction;
					break;
				}
			}
			
			ActionsManagerComponent actionsManagerBomb = ActionsManagerComponent.Cast(attachedEntity.FindComponent(ActionsManagerComponent));
			if (!actionsManagerBomb)
				return false;
			
			R3D_SetExplosionLiveAction setLiveAction;
			outActions = {};
			actionsManagerBomb.GetActionsList(outActions);
			foreach (BaseUserAction action : outActions)
			{
				R3D_SetExplosionLiveAction explosionAction = R3D_SetExplosionLiveAction.Cast(action);
				if (explosionAction)
				{
					setLiveAction = explosionAction;
					break;
				}
			}
			
			if (dropAction)
				actionPerformer.PerformAction(dropAction);
			
			if (setLiveAction)
				actionPerformer.PerformAction(setLiveAction);
			
			R3D_RocketMoveComponent rmc = R3D_RocketMoveComponent.Cast(attachedEntity.FindComponent(R3D_RocketMoveComponent));
			if (rmc)
			{
				rmc.Launch();
			}
			
			return true;
		}
		
		return false;
	}
}