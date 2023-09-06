class R3D_ExplosionTriggerComponentClass: ScriptComponentClass
{
}

class R3D_ExplosionTriggerComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Warhead Prefab", "et")]
	protected ResourceName m_sExplosionPrefab;
	
	[Attribute(defvalue: "-1", params: "-1 inf")]
	protected float m_fMinAltitudeAGLTrigger;
	
	[Attribute(defvalue: "-1", params: "-1 inf")]
	protected float m_fMinPressureTrigger;
	
	[Attribute()]
	protected bool m_bCollisionTrigger;
	
	[Attribute()]
	protected bool m_bBelowTerrainTrigger; // if item goes below terrain, should we just explode at ground level?
	
	[Attribute()]
	protected float m_fArmingDelay;
	
	protected ref array<IEntity> m_IgnoredCollisionEntities = {};
	protected bool m_bArmed = false;
	protected IEntity m_Owner = null;
	protected RplComponent m_RplComponent = null;
	
	void IgnoreEntityOnCollision(IEntity entity)
	{
		m_IgnoredCollisionEntities.Insert(entity);
	}
	
	void UnIgnoreEntityOnCollision(IEntity entity)
	{
		if (m_IgnoredCollisionEntities.Contains(entity))
		{
			m_IgnoredCollisionEntities.RemoveItem(entity);
		}			
	}
	
	void TriggerArmExplosive()
	{
		if (!m_sExplosionPrefab.Length() > 0) return;
		
		if (m_fArmingDelay > 0)
		{
			GetGame().GetCallqueue().CallLater(ArmExplosive, m_fArmingDelay * 1000);
		} else {
			ArmExplosive();
		}
	}
	
	void ArmExplosive()
	{
		if (!m_sExplosionPrefab.Length() > 0) return;
		
		int mask = EntityEvent.FRAME;
		if (m_bCollisionTrigger)
		{
			mask = mask | EntityEvent.CONTACT;
		}
		
		SetEventMask(GetOwner(), mask);
		
		m_bArmed = true;
	}
	
	void DearmExplosive()
	{
		if (!m_bArmed) return;
		
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
		if (m_bCollisionTrigger)
		{
			ClearEventMask(GetOwner(), EntityEvent.CONTACT);
		}
		
		m_bArmed = false;
	}
	
	private bool m_bHasExploded = false;
	void Explode()
	{
		if (m_bHasExploded) return;
		m_bHasExploded = true;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		m_Owner.GetTransform(params.Transform);
		
		Resource warheadResource = Resource.Load(m_sExplosionPrefab);
		if (!warheadResource) return;
		
		IEntity warhead = GetGame().SpawnEntityPrefab(warheadResource, null, params);
		BaseTriggerComponent warheadTrigger = BaseTriggerComponent.Cast(warhead.FindComponent(BaseTriggerComponent));
		if (warheadTrigger)
		{
			warheadTrigger.OnUserTrigger(m_Owner);
		}
		
		SCR_EntityHelper.DeleteEntityAndChildren(m_Owner);
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_Owner = owner;
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}	
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!m_bArmed || m_bHasExploded || m_RplComponent.IsProxy()) return;
		
		vector position = m_Owner.GetOrigin();
		float altitudeASL = position[1];
		bool triggered = false;
		
		if (m_fMinPressureTrigger > -1)
		{
			float pressure = ADM_InternationalStandardAtmosphere.GetValue(altitudeASL, ADM_ISAProperties.Pressure);
			if (pressure >= m_fMinPressureTrigger)
			{
				triggered = true;
			}
		}
		
		if (m_fMinAltitudeAGLTrigger > -1 || m_bBelowTerrainTrigger)
		{
			float terrainHeight = SCR_TerrainHelper.GetTerrainY(position);
			if (altitudeASL <= terrainHeight && m_bBelowTerrainTrigger)
			{
				position[1] = terrainHeight;
				m_Owner.SetOrigin(position);
				triggered = true;
			}
			
			if (m_fMinAltitudeAGLTrigger > -1 && altitudeASL <= (terrainHeight + m_fMinAltitudeAGLTrigger))
			{
				triggered = true;
			}
		}
		
		if (triggered) Explode();
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		super.EOnContact(owner, other, contact);
		
		if (!m_bCollisionTrigger || !m_bArmed || m_IgnoredCollisionEntities.Contains(other)) return;
			
		Explode();
	}
}