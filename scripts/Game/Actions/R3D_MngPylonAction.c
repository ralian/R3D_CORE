class R3D_MngPylonAction : ScriptedUserAction
{
	[Attribute(defvalue: "", desc: "Pylon in SlotManager that this action is for")]
	string m_sPylonSlot;
	protected bool m_bCache = false;
	
	R3D_PylonSlotInfo GetSlot()
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
		{
			return null;
		}
		
		R3D_PylonSlotInfo slot = R3D_PylonSlotInfo.Cast(slotManager.GetSlotByName(m_sPylonSlot));
		if (!slot)
		{
			return null;
		}
		
		return slot;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		R3D_PylonSlotInfo slot = GetSlot();
		
		if (slot.CanUnload())
		{
			IEntity entity = slot.GetAttachedEntity();
			if (entity)
			{
				R3D_PylonSlotInfo.DetachFromParent(entity); 
			}
				
			return;
		}
		
		// TODO: How to make sure everyone gets the same nearest loadable? had a case where two clients loaded different things
		IEntity entity = slot.NearestLoadable();
		if (entity)
		{
			slot.AttachEntity(entity);
		}
	}
	
	override bool CanBePerformedScript(IEntity user) 
	{ 
		return m_bCache; 
	}
	
	override bool GetActionNameScript(out string outName) 
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
		{
			return false;
		}
		
		R3D_PylonSlotInfo slot = R3D_PylonSlotInfo.Cast(slotManager.GetSlotByName(m_sPylonSlot));
		if (!slot)
		{
			return false;
		}
		
		if (slot.CanUnload())
		{
			m_bCache = true;
			outName = "Unload Armament";
			return true;
		}
		
		IEntity entitiy = slot.NearestLoadable();
		if (entitiy)
		{
			string name = entitiy.GetPrefabData().GetPrefabName();
			SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(entitiy.FindComponent(SCR_EditableEntityComponent));
			if (editableEntity) name = editableEntity.GetInfo().GetName();
			
			m_bCache = true;
			outName = "Load " + name;
			return true;
		}
		
		return true;
	}	
}