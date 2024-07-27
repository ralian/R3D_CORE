class ADM_AirplaneControllerComponentClass : CarControllerComponentClass
{
}

/*!
	Class responsible for game airplane.
	It connects all airplane components together and handles all comunication between them.
*/
class ADM_AirplaneControllerComponent: CarControllerComponent
{
	[RplProp()] 
	protected bool m_bIsEngineOn = false; //Set the engine to be off by default
	
	[RplProp()] 
	protected bool m_bGearDeployed = true; // true = deployed, false = retracted
	
	[Attribute(category: "Airplane", desc: "[km/h, deg/s, -]")]
	protected ref array<vector> m_fSteeringStrength;
	
	protected ref array<ADM_EngineComponent> m_Engines = {};
	protected ADM_FixedWingSimulation m_FixedWingSim;
	protected ADM_AirplaneInput m_AirplaneInput;
	protected RplComponent m_RplComponent = null;
	
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
		m_AirplaneInput = ADM_AirplaneInput.Cast(owner.FindComponent(ADM_AirplaneInput));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	ADM_FixedWingSimulation GetFixedWingSimulation()
	{
		return m_FixedWingSim;
	}
	
	//------------------------------------------------------------------------------------------------
	ADM_AirplaneInput GetAirplaneInput()
	{
		return m_AirplaneInput;
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
		Print("ToggleGear");
		Rpc(Rpc_Owner_ToggleGear);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Owner_ToggleGear()
	{
		Print("Rpc_Owner_ToggleGear");
		m_bGearDeployed = !m_bGearDeployed;
		Replication.BumpMe();
		
		if (!m_FixedWingSim)
			return;
		
		foreach (ADM_LandingGear gear: m_FixedWingSim.GetGear())
		{
			gear.SetGearDeployed(m_bGearDeployed);
		}
	}
	
}