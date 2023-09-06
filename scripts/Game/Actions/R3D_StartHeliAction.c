class R3D_StartHeliAction : SCR_ScriptedUserAction
{	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		VehicleHelicopterSimulation vhs = VehicleHelicopterSimulation.Cast(GetOwner().FindComponent(VehicleHelicopterSimulation));
		
		if (!vhs || vhs.EngineIsOn())
			vhs.EngineStop();
		else 
			vhs.EngineStart();
	}
	
	override bool GetActionNameScript(out string outName)
	{
		VehicleHelicopterSimulation vhs = VehicleHelicopterSimulation.Cast(GetOwner().FindComponent(VehicleHelicopterSimulation));
		
		if (!vhs || vhs.EngineIsOn())
			outName = "Stop Engine";
		else 
			outName = "Start Engine";
		
		return true;
	};
	
	override bool CanBePerformedScript(IEntity user)
	{
		VehicleHelicopterSimulation vhs = VehicleHelicopterSimulation.Cast(GetOwner().FindComponent(VehicleHelicopterSimulation));
		if (!vhs) return false;
		
		return true;
	}
};
