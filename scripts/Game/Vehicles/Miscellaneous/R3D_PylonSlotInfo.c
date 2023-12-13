class R3D_PylonSlotInfo: EntitySlotInfo
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "List of resources", "et")]
	protected ref array<ResourceName> m_sAllowedPrefabs;
	
	[Attribute(desc: "Distance to look for attachable items", defvalue: "5")]
	protected float m_fSearchDistance;
	
	//------------------------------------------------------------------------------------------------
	//! Checks whether provided entity has parent and if so, tries to find a slot which it would belong to
	//! and unregister from it.
	// SCR_DestructionHandler::DetachFromParent
	static void DetachFromParent(IEntity entity)
	{
		if (!entity || entity.IsDeleted())
			return;

		IEntity parent = entity.GetParent();
		if (!parent || parent.IsDeleted())
			return;

		Physics physics = entity.GetPhysics();
		if (!physics)
			return;

		//TODO: replace with get velocity at and get angular velocity at position if the plane is rotating
		vector velocity = entity.GetRootParent().GetPhysics().GetVelocity();
		vector angularVelocity = entity.GetRootParent().GetPhysics().GetAngularVelocity(); 

		EntitySlotInfo slotInfo = EntitySlotInfo.GetSlotInfo(entity);
		if (slotInfo)
		{
			slotInfo.DetachEntity(false); // for some reason updateHierarchy = true will crash, instead lets just remove it manually
		}
			
		if (parent)
		{
			parent.RemoveChild(entity, true);
		}
		
		parent.Update();
		entity.Update();
			
		if (physics.IsDynamic() && !entity.GetParent())
		{
			physics.SetVelocity(velocity);
			physics.SetAngularVelocity(angularVelocity);
		}
	}
	
	bool CanLoad(IEntity entity)
	{
		bool alreadyOnPylon = false;
		IEntity parent = entity.GetParent();
		if (parent)
			alreadyOnPylon = true;
		
		return ((GetAttachedEntity() == null) && entity && entity.GetParent() != GetOwner() && !alreadyOnPylon && entity.GetPrefabData() && m_sAllowedPrefabs.Contains(entity.GetPrefabData().GetPrefabName()));
	}
	
	bool CanUnload()
	{
		return (GetAttachedEntity() != null);
	}
	
	ref array<IEntity> m_NearItems = {};
	bool LoadableFilter(IEntity ent)
	{
		if (CanLoad(ent))
		{
			m_NearItems.Insert(ent);
		}
			
		return true;
	}
	
	IEntity NearestLoadable()
	{
		m_NearItems.Clear();
		
		vector mat[4];
		vector search = vector.One * m_fSearchDistance;
		GetWorldTransform(mat);
		GetGame().GetWorld().QueryEntitiesByAABB(mat[3] - search, mat[3] + search, LoadableFilter);
		
		IEntity nearestValid = null;
		float distSq = float.MAX;
		foreach (IEntity ent : m_NearItems)
		{
			if (vector.DistanceSq(ent.GetOrigin(), mat[3]) < distSq)
			{
				nearestValid = ent;
				distSq = vector.DistanceSq(ent.GetOrigin(), mat[3]);
			}
		}
		
		return nearestValid;
	}
	
	bool TriggerPylon()
	{
		IEntity item = GetAttachedEntity();
		if (!item) 
		{
			return false;
		}
		
		R3D_PylonTriggerComponent pylonTriggerComp = R3D_PylonTriggerComponent.Cast(item.FindComponent(R3D_PylonTriggerComponent));
		if (!pylonTriggerComp)
		{
			Print("Pylon has no trigger component!");
			return false;
		}
		
		pylonTriggerComp.Trigger(this);
		return true;
	}
	
	override void OnAttachedEntity(IEntity entity)
	{
		if (!m_sAllowedPrefabs.Contains(entity.GetPrefabData().GetPrefabName()))
		{
			DetachEntity(true);
			return;
		}
		
		super.OnAttachedEntity(entity);
		
		R3D_DynamicMassComponent dynamicMass = R3D_DynamicMassComponent.Cast(GetOwner().FindComponent(R3D_DynamicMassComponent));
		if (dynamicMass)
		{
			dynamicMass.UpdateRecursiveMassProperties();
		}
	}
	
	override void OnDetachedEntity(IEntity entity)
	{
		super.OnDetachedEntity(entity);
		
		R3D_DynamicMassComponent dynamicMass = R3D_DynamicMassComponent.Cast(GetOwner().FindComponent(R3D_DynamicMassComponent));
		if (dynamicMass)
		{
			dynamicMass.UpdateRecursiveMassProperties();
		}
	}
}