class R3D_WeaponRecoilComponentClass: ScriptComponentClass
{
}

class R3D_WeaponRecoilComponent : ScriptComponent
{
	[Attribute()]
	protected int m_iWeaponIndex;
	
	[Attribute()]
	protected float m_fRecoilForce;
	
	[Attribute()]
	protected ref PointInfo m_vRecoilPosition;
	
	protected IEntity m_Owner = null;
	protected Physics m_Physics = null;
	protected BaseMuzzleComponent m_Muzzle = null;
	private int m_iPreviousAmmoCount = 0;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_Owner = owner;
		m_Physics = owner.GetPhysics();
		m_vRecoilPosition.Init(owner);
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.SIMULATE);
	}
	
	override void EOnInit(IEntity owner)
	{
		// Must be done in EOnInit and not OnPostInit so weapons are created
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager) return;
		
		array<WeaponSlotComponent> weaponSlots = {};
		weaponManager.GetWeaponsSlots(weaponSlots);
		
		foreach(WeaponSlotComponent weaponSlot : weaponSlots)
		{
			if (weaponSlot.GetWeaponSlotIndex() == m_iWeaponIndex || weaponSlots.Count() <= 1)
			{
				IEntity weapon = weaponSlot.GetWeaponEntity();
				if (!weapon) return;
				m_Muzzle = BaseMuzzleComponent.Cast(weapon.FindComponent(BaseMuzzleComponent));
				m_iPreviousAmmoCount = m_Muzzle.GetAmmoCount();
				
				return;
			}
		}
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_Physics || !m_Muzzle) return;
		
		int currentAmmoCount = m_Muzzle.GetAmmoCount();
		int deltaAmmo = currentAmmoCount - m_iPreviousAmmoCount;
		
		if (deltaAmmo < 0)
		{
			vector originMat[4];			
			m_vRecoilPosition.GetModelTransform(originMat);
			
			vector position = owner.CoordToParent(originMat[3]);
			vector force = owner.VectorToParent(originMat[2]) * -1 * m_fRecoilForce;
			m_Physics.ApplyImpulseAt(position, force * timeSlice);
		}
		
		m_iPreviousAmmoCount = currentAmmoCount;
	}
}