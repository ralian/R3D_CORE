class R3D_LoopedGunSoundClass: ScriptGameComponentClass {}

class R3D_LoopedGunSound : ScriptGameComponent
{	
	[Attribute()]
	protected int m_iWeaponIndex;
	
	private SignalsManagerComponent m_WeaponSignalsManager;
	protected BaseMuzzleComponent m_Muzzle = null;
	private int m_iPreviousAmmoCount = 0;
	private int m_iWeaponSignal = -1;
	private int m_iWeaponWasFiringSignal = -1;
	private float m_fLastFireTime = -500;
	private float m_fReleaseTime = -500;
	private bool m_bWasFiring = false;
	
	override event protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
				
		m_WeaponSignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_Muzzle = BaseMuzzleComponent.Cast(owner.FindComponent(BaseMuzzleComponent));
		m_iPreviousAmmoCount = m_Muzzle.GetAmmoCount();
				
		if (m_WeaponSignalsManager)
		{
			m_iWeaponSignal = m_WeaponSignalsManager.AddOrFindMPSignal("VehicleFire", 0.1, 1/30, 0, SignalCompressionFunc.Range01);
			m_iWeaponWasFiringSignal = m_WeaponSignalsManager.AddOrFindMPSignal("VehicleFireReleased", 0.1, 1/30, 0, SignalCompressionFunc.Range01);
		}
				
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	override event protected bool OnTicksOnRemoteProxy() 
	{
		return false; 
	}
	
	override event protected void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (m_Muzzle && m_WeaponSignalsManager) 
		{
			int currentAmmoCount = m_Muzzle.GetAmmoCount();
			int deltaAmmo = currentAmmoCount - m_iPreviousAmmoCount;
			
			bool vehicleFire = deltaAmmo < 0 || (System.GetTickCount() - m_fLastFireTime) < 150;
			bool vehicleFireReleased = !vehicleFire && m_bWasFiring;
			if (vehicleFireReleased)
			{
				m_fReleaseTime = System.GetTickCount();
			}
			
			m_WeaponSignalsManager.SetSignalValue(m_iWeaponSignal, (int)vehicleFire);
			m_WeaponSignalsManager.SetSignalValue(m_iWeaponWasFiringSignal, (System.GetTickCount() - m_fReleaseTime) < 6000 && (System.GetTickCount() - m_fLastFireTime) > 50);
			
			if (deltaAmmo < 0)
			{
				m_fLastFireTime = System.GetTickCount();
			}
				
			m_iPreviousAmmoCount = currentAmmoCount;
			m_bWasFiring = vehicleFire;
		}
	}
}