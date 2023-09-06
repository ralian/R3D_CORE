class R3D_RadarCrossSectionComponentClass : ScriptComponentClass
{
}

class R3D_RadarCrossSectionComponent: ScriptComponent
{
	[Attribute(desc: "RCS ellipsoid extents in m^2")]
	vector RCS_area;
	
	static ref array<IEntity> lsRCS = {};
	
	// Return the effective radar cross section from a viewing position, in m^2
	float GetEffectiveRCSFrom(vector worldPos) {
		vector tform[4];
		vector contributions;
		
		GetOwner().GetWorldTransform(tform);
		
		vector direction = worldPos - tform[3];
		direction.Normalize();
		
		contributions[0] = vector.Dot(tform[0], direction);
		contributions[1] = vector.Dot(tform[1], direction);
		contributions[2] = vector.Dot(tform[2], direction);
		
		return vector.Dot(contributions, RCS_area);
	}
	
	// Return the effective speed towards a position in m/s. Positive is closing.
	float GetSpeedTowards(vector worldPos) {
		vector velWorld = GetOwner().GetPhysics().GetVelocity();
		
		vector direction = worldPos - GetOwner().GetOrigin();
		direction.Normalize();
		
		return vector.Dot(direction, velWorld);
	}
	
	override event protected void EOnInit(IEntity owner) {
		if (Replication.IsServer()) {
			R3D_RadarCrossSectionComponent.lsRCS.Insert(owner);
		}
	}
}
