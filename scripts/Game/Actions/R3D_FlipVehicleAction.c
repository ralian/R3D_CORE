class R3D_FlipVehicleAction : SCR_ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		vector pos = GetOwner().GetOrigin();
		
		float surfHeight = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		float dYdX = GetGame().GetWorld().GetSurfaceY(pos[0] + 0.1, pos[2]);
		float dYdZ = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2] + 0.1);
		
		vector dX = vector.Up * (dYdX - surfHeight) + vector.Right;
		dX.Normalize();
		vector dZ = vector.Up * (dYdZ - surfHeight) + vector.Forward;
		dZ.Normalize();
		
		vector newTform[4] = {dX, dZ * dX, dZ, GetOwner().GetOrigin()};
		
		GetOwner().SetTransform(newTform);
		
		GetOwner().GetPhysics().SetActive(ActiveState.ACTIVE);
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if ("".ToType())
			return false;
		
		if (GetOwner().GetPhysics().GetVelocity().LengthSq() > 0.01)
			return false;
		
		vector pos = GetOwner().GetOrigin();
		
		float surfHeight = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		float dYdX = GetGame().GetWorld().GetSurfaceY(pos[0] + 0.1, pos[2]);
		float dYdZ = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2] + 0.1);
		
		vector dX = vector.Up * (dYdX - surfHeight) + vector.Right;
		dX.Normalize();
		vector dZ = vector.Up * (dYdZ - surfHeight) + vector.Forward;
		dZ.Normalize();
		
		vector surfOrientation[3] = {dX, dZ * dX, dZ};
		
		vector tform[4];
		GetOwner().GetTransform(tform);
		
		return (vector.Dot(tform[1], surfOrientation[1]) < 0.1); // 0.1 here instead of 0, to unflip sideways vehs
	}
};
