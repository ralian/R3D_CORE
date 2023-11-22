class R3D_PylonComponentClass : ScriptComponentClass {}

class R3D_PylonComponent : ScriptComponent {
	[Attribute(defvalue: "-1")]
	int pylonNo;
	
	[Attribute()]
	ref PointInfo pylonOrigin;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "List of resources", "et")]
	ref array<ResourceName> allowedPrefabs;
	
	IEntity m_owner = null;
	Physics m_physics = null;
	IEntity item = null;
	
	override void OnPostInit(IEntity owner)
	{
		m_owner = owner;
		m_physics = owner.GetPhysics();
		pylonOrigin.Init(owner);
	}
	
	void ResetPylon()
	{
		R3D_DynamicMassComponent dynamicMass = R3D_DynamicMassComponent.Cast(m_owner.FindComponent(R3D_DynamicMassComponent));
		if (dynamicMass)
		{
			dynamicMass.UpdateRecursiveMassProperties();
		}
		
		item = null;
	}
	
	bool TriggerPylon()
	{
		if (!item) return false;
		
		//R3D_PylonTriggerComponent pylonTrigger = R3D_PylonTriggerComponent.Cast(item.FindComponent(R3D_PylonTriggerComponent));
		//if (pylonTrigger)
		//{
		//	return pylonTrigger.Trigger(m_owner, this);
		//}
		
		return false;
	}
	
	ref array<IEntity> nearItems = {};
	bool LoadableFilter(IEntity ent)
	{
		if (CanLoadItem(ent))
			nearItems.Insert(ent);
		
		return true;
	}
	
	IEntity NearestLoadable()
	{
		nearItems.Clear();
		
		vector originMat[4];
		if (m_owner.GetAnimation())
			m_owner.GetAnimation().GetBoneMatrix(pylonOrigin.GetNodeId(), originMat);
		
		vector originPos = m_owner.CoordToParent(originMat[3]);
		GetGame().GetWorld().QueryEntitiesByAABB(originPos - "5 5 5", originPos + "5 5 5", LoadableFilter);
		
		IEntity nearestValid = null;
		float distSq = float.MAX;
		foreach (IEntity ent : nearItems)
		{
			if (vector.DistanceSq(ent.GetOrigin(), originPos) < distSq)
			{
				nearestValid = ent;
				distSq = vector.DistanceSq(ent.GetOrigin(), originPos);
			}
		}
		
		return nearestValid;
	}
	
	bool CanLoadItem(IEntity entity)
	{
		// If any parent has R3D_PylonComponent then the entity is already loaded on to a pylon and we can't use it
		bool alreadyOnPylon = false;
		IEntity parent = entity.GetParent();
		while (parent)
		{
			R3D_PylonComponent pylon = R3D_PylonComponent.Cast(parent.FindComponent(R3D_PylonComponent));
			if (pylon)
			{
				alreadyOnPylon = true;
				break;
			}
			parent = parent.GetParent();
		}
		
		return (!item && entity && entity.GetParent() != m_owner && !alreadyOnPylon && entity.GetPrefabData() && allowedPrefabs.Contains(entity.GetPrefabData().GetPrefabName()));
	}
	
	bool CanUnloadItem()
	{
		return item;
	}
	
	bool LoadItem(IEntity entity)
	{
		if (item) return false;
		
		m_owner.AddChild(entity, -1, EAddChildFlags.AUTO_TRANSFORM);
		
		vector pointMat[4];
		pylonOrigin.GetModelTransform(pointMat);
		entity.SetLocalTransform(pointMat);
		
		if (entity.GetPhysics())
		{
			entity.GetPhysics().EnableGravity(false);
			entity.GetPhysics().SetActive(false);
		}
		
		R3D_DynamicMassComponent dynamicMass = R3D_DynamicMassComponent.Cast(m_owner.FindComponent(R3D_DynamicMassComponent));
		if (dynamicMass)
		{
			dynamicMass.UpdateRecursiveMassProperties();
		}
		
		item = entity;
		return true;
	}
	
	bool UnloadItem()
	{
		if (!item || item.GetParent() != m_owner) //return false; (if we don't have an item and it's parent is not this pylon, we should just reset the state variables so this pylon doesn't become bugged
		{
			ResetPylon();
			return false;
		}
		
		vector transform[4];
		item.GetWorldTransform(transform);
		m_owner.RemoveChild(item);
		item.SetWorldTransform(transform);
		
		vector velocity = m_owner.GetRootParent().GetPhysics().GetVelocityAt(item.GetOrigin());
		if (item.GetPhysics())
		{
			item.GetPhysics().EnableGravity(true);
			item.GetPhysics().SetActive(true);
			item.GetPhysics().SetVelocity(velocity);
		}
		
		ResetPylon();
		return true;
	}
}