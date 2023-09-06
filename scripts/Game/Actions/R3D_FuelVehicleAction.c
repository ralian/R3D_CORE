class R3D_FuelVehicleAction : ScriptedUserAction
{
	[Attribute()]
	bool remove_after_use;
	
	ref array<Vehicle> nearVehicles = {};
	
	protected bool AddRefuelTarget(IEntity entity)
	{
		Vehicle veh = Vehicle.Cast(entity);
		if (!veh) return true;
		
		FuelManagerComponent fmc = FuelManagerComponent.Cast(veh.FindComponent(FuelManagerComponent));
		if (!fmc) return true;
		
		float fuel_pct = fmc.GetTotalFuel() / fmc.GetTotalMaxFuel();
		if (fuel_pct > 0.99) return true;
		
		nearVehicles.Insert(veh);
		
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		vector pos = GetOwner().GetOrigin();
		FuelManagerComponent nearest = null;
		float dist_min = float.MAX;
		
		foreach (Vehicle veh : nearVehicles)
		{
			FuelManagerComponent fmc = FuelManagerComponent.Cast(veh.FindComponent(FuelManagerComponent));
			if (!fmc) continue;
			
			float dist = vector.DistanceSq(veh.GetOrigin(), pos);
			if (dist < dist_min)
			{
				nearest = fmc;
				dist_min = dist;
			}	
		}
			
		array<BaseFuelNode> fnodes = {};
			
		nearest.GetFuelNodesList(fnodes);
			
		foreach (BaseFuelNode fn : fnodes)
		{
			fn.SetFuel(fn.GetMaxFuel());
		}
		
		if (remove_after_use)
			SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		nearVehicles.Clear();
		vector pos = GetOwner().GetOrigin();
		GetGame().GetWorld().QueryEntitiesByAABB(pos - "5 5 5", pos + "5 5 5", AddRefuelTarget);
		return (nearVehicles.Count() > 0);
	}
};
