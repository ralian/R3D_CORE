class R3D_MngPylonAction : ScriptedUserAction
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
				if (pyCmp.CanUnloadItem())
				{
					pyCmp.UnloadItem();
					return;
				}
				
				IEntity it = pyCmp.NearestLoadable();
				if (it)
					pyCmp.LoadItem(it);
			}
		}
	}
	
	override bool CanBePerformedScript(IEntity user) { return viCache; };
	
	override bool GetActionNameScript(out string outName) {
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
					outName = "Unload Armament";
					return true;
				}
				
				IEntity it = pyCmp.NearestLoadable();
				if (it)
				{
					string name = it.GetPrefabData().GetPrefabName();
					R3D_UIInfo uiInfo = R3D_UIInfo.Cast(it.FindComponent(R3D_UIInfo));
					if (uiInfo) name = uiInfo.GetUIInfo().GetName();
					
					viCache = true;
					outName = "Load " + name;
					return true;
				}
			}
		}
		
		viCache = false;
		outName = "No Item to Load";
		return true;
	}	
}