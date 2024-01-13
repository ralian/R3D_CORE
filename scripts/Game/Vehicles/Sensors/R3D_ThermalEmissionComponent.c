class R3D_ThermalEmitterComponentClass : ScriptComponentClass
{
}

class R3D_ThermalEmitterComponent: ScriptComponent
{
	[Attribute(desc: "The position of this particular heat signature. You can have multiple in different locations!!!")]
	vector origin;
	
	[Attribute(desc: "Maximum temperature (K) when the engines are operating at full power/burner", defvalue: "1073")]
	float T_max_K;
	
	[Attribute(desc: "Temperature (K) when the engines are operating near idle", defvalue: "673")]
	float T_nominal_K;
	
	[Attribute(desc: "Time constant (s) for the dumb simulation of heat transfer", defvalue: "10")]
	float t_heat_transfer;
	
	float T_current = 274.5;
	// = GetGame().GetTimeAndWeatherManager().GetAmbientTemperature(); // someday
	
	override void EOnSimulate(IEntity owner, float timeSlice) {
		bool power_on = false; 
		float max_heat_fraction = 0.0;
		
		auto vehSim = VehicleBaseSimulation.Cast(owner.FindComponent(VehicleBaseSimulation));
        typename simType = vehSim.Type();
        
        switch(simType)
        {
            case VehicleWheeledSimulation:
                power_on = VehicleWheeledSimulation.Cast(vehSim).EngineIsOn();
                break;
            case VehicleWheeledSimulation_SA:
                power_on = VehicleWheeledSimulation_SA.Cast(vehSim).EngineIsOn();
                break;
            case VehicleWheeledSimulation_SA_B:
                power_on = VehicleWheeledSimulation_SA_B.Cast(vehSim).EngineIsOn();
                break;
            case VehicleHelicopterSimulation:
                power_on = VehicleHelicopterSimulation.Cast(vehSim).EngineIsOn();
                break;
			// TODO
            //case ADM_FixedWingSimulation:
            //    power_on = ADM_FixedWingSimulation.Cast(vehSim).IsEngineOn();
            //    break;
            default: 
                Print("Unsupported vehicle simulation type"); // Or return some default value? (for flares)
                break;
        }
		
		float target_temp = 274.5;
		if (power_on) target_temp = T_nominal_K + max_heat_fraction*(T_max_K - T_nominal_K);
		
		T_current = T_current + (target_temp - T_current) * (timeSlice / t_heat_transfer);
	}
}
