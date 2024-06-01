class ADM_AirplaneControllerComponent_SAClass : SCR_CarControllerComponent_SAClass
{
}

/*!
	Class responsible for game airplane.
	It connects all airplane components together and handles all comunication between them.
*/
class ADM_AirplaneControllerComponent_SA: SCR_CarControllerComponent_SA
{
	[RplProp()] 
	protected bool m_bIsEngineOn = false; //Set the engine to be off by default
	
	[RplProp()] 
	protected bool m_bGearDeployed = true; // true = deployed, false = retracted
	
	protected ref array<ADM_EngineComponent> m_Engines = {};
	protected ADM_FixedWingSimulation m_FixedWingSim;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		array<Managed> components = {};
		owner.FindComponents(ADM_EngineComponent, components);
		foreach (Managed cmp : components)
		{
			ADM_EngineComponent engCmp = ADM_EngineComponent.Cast(cmp);
			if (engCmp) m_Engines.Insert(engCmp);
		}
		
		m_FixedWingSim = ADM_FixedWingSimulation.Cast(owner.FindComponent(ADM_FixedWingSimulation));
	}
	
	//------------------------------------------------------------------------------------------------
	ADM_FixedWingSimulation GetFixedWingSimulation()
	{
		return m_FixedWingSim;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ADM_EngineComponent> GetEngines()
	{
		return m_Engines;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAirplaneEngineOn()
	{
		return m_bIsEngineOn;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Server_ToggleEngine()
	{
		m_bIsEngineOn = !m_bIsEngineOn;
		Replication.BumpMe();
		
		foreach(ADM_EngineComponent engine : m_Engines)
		{
			engine.SetEngineStatus(m_bIsEngineOn);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleGear()
	{
		Rpc(Rpc_Owner_ToggleGear);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Owner_ToggleGear()
	{
		m_bGearDeployed = !m_bGearDeployed;
		
		// TODO: only do this if all gear retract
		/*if (!m_bGearState)
		{
			m_VehicleBaseSim.Deactivate(m_Owner);
			m_Physics.EnableGravity(true);
		} else {
			m_VehicleBaseSim.Activate(m_Owner);
		}*/
			
		Replication.BumpMe();
	}
}