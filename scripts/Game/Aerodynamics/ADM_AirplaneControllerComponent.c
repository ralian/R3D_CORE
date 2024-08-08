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

	[Attribute(category: "Fixed Wing Simulation", defvalue: "0.01")]
	float m_fThrustVelocity;
	
	[Attribute(category: "Fixed Wing Simulation", desc: "Return steering to zero when no input applied.", defvalue: "1")]
	bool m_bAutoZeroSteerAngle;
	
	[Attribute(category: "Fixed Wing Simulation", desc: "Max angle defined in vehicle wheeled simulation steering component. THIS MUST MATCH FOR STEERING STRENGTH TO BE MAPPED CORRECTLY!")]
	float m_fMaxSteerAngle;
	
	[Attribute(category: "Fixed Wing Simulation", uiwidget: UIWidgets.Auto, params: "100 100 0 0", desc: "Steering speed when actively steering away from center [km/h deg/s]")]
	ref Curve m_fSteeringForwardStrength;
	
	[Attribute(category: "Fixed Wing Simulation", uiwidget: UIWidgets.Auto, params: "100 100 0 0", desc: "Steering speed when actively steering away toward center [km/h deg/s]")]
	ref Curve m_fSteeringBackwardStrength;
	
	[Attribute(category: "Fixed Wing Simulation", uiwidget: UIWidgets.Auto, params: "100 100 0 0", desc: "Steering speed when no steering applied [km/h deg/s]")]
	ref Curve m_fSteeringCenterStrength;
	
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
		Rpc(Rpc_Server_ToggleGear);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_Server_ToggleGear()
	{
		Print("Rpc_Server_ToggleGear");
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