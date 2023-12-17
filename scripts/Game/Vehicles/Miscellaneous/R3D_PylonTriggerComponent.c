class R3D_PylonTriggerComponentClass : ScriptComponentClass {}

class R3D_PylonTriggerComponent : ScriptComponent 
{
	// Mod me for custom behavior on drop. (like triggering a missile)
	bool Trigger(R3D_PylonSlotInfo fromPylon)
	{
		Print("a");
		if (fromPylon.CanUnload())
		{
			IEntity attachedEntity = fromPylon.GetAttachedEntity();
			if (!attachedEntity)
				return false;
			
			Print("b");
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (!playerController)
				return false;
			
			Print("c");
			IEntity player = playerController.GetMainEntity();	
			if (!player)
				return false;
			
			Print("d");
			ActionsPerformerComponent actionPerformer = ActionsPerformerComponent.Cast(player.FindComponent(ActionsPerformerComponent));
			if (!actionPerformer)
				return false;
			Print("e");
			
			ActionsManagerComponent actionsManagerPylon = ActionsManagerComponent.Cast(fromPylon.GetOwner().FindComponent(ActionsManagerComponent));
			if (!actionsManagerPylon)
				return false;
			Print("f");
			
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
			Print("h");
			
			ActionsManagerComponent actionsManagerBomb = ActionsManagerComponent.Cast(attachedEntity.FindComponent(ActionsManagerComponent));
			if (!actionsManagerBomb)
				return false;
			
			Print("i");
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
			Print("j");
			
			if (dropAction)
				actionPerformer.PerformAction(dropAction);
			
			Print("k");
			if (setLiveAction)
				actionPerformer.PerformAction(setLiveAction);
			
			Print("l");
			return true;
		}
		
		return false;
	}
}