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
			
			if (attachedEntity)
			{
				//Print("calling rpc");
				//RplComponent rplComp = RplComponent.Cast(attachedEntity.FindComponent(RplComponent));
				//if (!rplComp)
				//{
				//	return false;
				//}
				
				//Rpc(Rpc_DoSetLive, rplComp.Id());
				/*BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(attachedEntity.FindComponent(BaseTriggerComponent));
				if (triggerComp) 
				{
					triggerComp.SetLive();
				}*/
			}
			
			return true;
		}
		
		return false;
	}
	
	void SetLive(IEntity entity)
	{
		BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(entity.FindComponent(BaseTriggerComponent));
		if (triggerComp) 
		{
			triggerComp.SetLive();
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_DoSetLive(RplId entityId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(entityId));
		IEntity entity = rplComp.GetEntity();
		Print(entity);
		if (entity)
		{
			SetLive(entity);
			Rpc(Rpc_Broadcast_SetLive, entityId);
			Print("boom");
		}
	}
	
	[RplRpc(RplRcver.Broadcast, RplChannel.Reliable)]
	void Rpc_Broadcast_SetLive(RplId entityId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(entityId));
		IEntity entity = rplComp.GetEntity();
		Print(entity);
		if (entity)
		{
			SetLive(entity);
			Print("boom");
		}
	}
}