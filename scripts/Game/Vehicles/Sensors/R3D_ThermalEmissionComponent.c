class R3D_ThermalEmitterComponentClass : ScriptComponentClass
{
}

class R3D_ThermalEmitterComponent: ScriptComponent
{
	[Attribute(desc: "The position of this particular heat signature. You can have multiple in different locations!", uiwidget: UIWidgets.EditBox, params: "inf inf 0 purpose=coords space=entity")]
	vector origin;
	
	[Attribute(desc: "Heat emission radius", defvalue: "1")]
	float radius;
	
	[Attribute(desc: "Maximum temperature (K) when the engines are operating at full power/burner", defvalue: "1073")]
	float T_max_K;
	
	[Attribute(desc: "Temperature (K) when the engines are operating near idle", defvalue: "673")]
	float T_nominal_K;
	
	[Attribute(desc: "Time constant (s) for the dumb simulation of heat transfer", defvalue: "10")]
	float t_heat_transfer;
	
	float T_current = 274.5;
	// = GetGame().GetTimeAndWeatherManager().GetAmbientTemperature(); // someday
	
	static bool Filter(IEntity ent) {
		return ent.FindComponent(R3D_ThermalEmitterComponent);
	}
	
	override void OnPostInit(IEntity owner) {
		SetEventMask(owner, EntityEvent.SIMULATE);
		#ifdef WORKBENCH
		ConnectToDiagSystem(owner);
		#endif // WORKBENCH
	}
	
	void _DbgVisualize(IEntity owner, float timeSlice) {
		Shape.CreateSphere(COLOR_GREEN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, owner.CoordToParent(origin), radius);
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice) {
		// todo some logic here to determine the vehicle engine state 
		bool power_on = false; 
		bool max_power = false;
		
		float target_temp = 274.5;
		if (power_on) target_temp = T_nominal_K;
		if (max_power) target_temp = T_max_K;
		
		T_current = T_current + (target_temp - T_current) * (timeSlice / t_heat_transfer);
	}
	
	override void EOnDiag(IEntity owner, float timeSlice) {
		_DbgVisualize(owner, timeSlice);
	}
	
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice) {
		_DbgVisualize(owner, timeSlice);
	}
}
