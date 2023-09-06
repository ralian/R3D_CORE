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
		// todo some logic here to determine the vehicle engine state 
		bool power_on = false; 
		bool max_power = false;
		
		float target_temp = 274.5;
		if (power_on) target_temp = T_nominal_K;
		if (max_power) target_temp = T_max_K;
		
		T_current = T_current + (target_temp - T_current) * (timeSlice / t_heat_transfer);
	}
}
