class R3D_TriggerPylonAction : ScriptedUserAction
{
	[Attribute(defvalue: "-1")]
	int pylonNo;
	
	bool viCache = false;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		array<Managed> components = {};
		GetOwner().FindComponents(R3D_PylonComponent, components);
		
		foreach (Managed cmp : components)
		{
			R3D_PylonComponent pyCmp = R3D_PylonComponent.Cast(cmp);
			
			if (pyCmp && pyCmp.pylonNo == pylonNo)
			{
				if (pyCmp.TriggerPylon())
					return;
			}
		}
	}
	
	override bool CanBePerformedScript(IEntity user) 
	{ 
		return viCache; 
	}
	
	override bool GetActionNameScript(out string outName) 
	{
		array<Managed> components = {};
		GetOwner().FindComponents(R3D_PylonComponent, components);
		
		foreach (Managed cmp : components)
		{
			R3D_PylonComponent pyCmp = R3D_PylonComponent.Cast(cmp);
			
			if (pyCmp && pyCmp.pylonNo == pylonNo)
			{
				if (pyCmp.CanUnloadItem())
				{
					viCache = true;
					outName = "Trigger Pylon";
					return true;
				}
			}
		}
		
		viCache = false;
		outName = "Nothing to trigger";
		return true;
	}	
}