class R3D_ThermalReceiverComponentClass : ScriptComponentClass
{
}

class R3D_ThermalReceiverComponent: ScriptComponent
{
	[Attribute(desc: "Receiver position in local space")]
	vector origin;
	
	[Attribute(desc: "Start azumith of the search volume in deg", defvalue: "-45")]
	float azumith_min;
	
	[Attribute(desc: "End azumith of the search volume in deg", defvalue: "45")]
	float azumith_max;
	
	[Attribute(desc: "Start elev of the search volume in deg", defvalue: "-45")]
	float elev_min;
	
	[Attribute(desc: "End elev of the search volume in deg", defvalue: "45")]
	float elev_max;
	
	[Attribute(desc: "Seeker angle acceptable error in deg", defvalue: "1")]
	float angle_error;
	
	float azumith_current = 0;
	float elev_current = 0;
	
	R3D_ThermalEmitterComponent target = null;
	
	vector GetWSBoresightVector() {
		vector tform[3];
		GetOwner().GetWorldTransform(tform);
		
		vector seeker_tform[3];
		Math3D.AnglesToMatrix(Vector(azumith_current, elev_current, 0), seeker_tform);
		
		vector result[3];
		Math3D.MatrixMultiply3(tform, seeker_tform, result);
		
		return result[1]; 
	}
	
	R3D_ThermalEmitterComponent GrabBestTarget() {
		array<IEntity> entities = {};
		GetGame().GetWorld().GetActiveEntities(entities);
		
		vector tform[4];
		GetOwner().GetWorldTransform(tform);
		
		vector boresight_dir = GetWSBoresightVector();
		
		R3D_ThermalEmitterComponent hottest = null;
		foreach (IEntity ent : entities) {
			array<Managed> emitters = null;
			ent.FindComponents(R3D_ThermalEmitterComponent, emitters);
			
			foreach (Managed entry : emitters) {
				auto emitter = R3D_ThermalEmitterComponent.Cast(entry);
				if (emitter) {
					vector direction = (ent.VectorToParent(emitter.origin) - tform[3]).Normalized();
					if (vector.Dot(direction, boresight_dir) > Math.Cos(Math.DEG2RAD * angle_error))
						if (!hottest || hottest.T_current < emitter.T_current)
							hottest = emitter;
				}
			}
		}
		
		return hottest;
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice) {
		if (!target) return;
		
		// Make sure the current target is within the gimbal limits
		vector targetLS = owner.VectorToLocal(target.GetOwner().CoordToParent(target.origin)) - origin;
		
		vector tAng = targetLS.VectorToAngles();
		
		if (tAng[0] > 180) tAng[0] = tAng[0] - 360;
		
		if (tAng[0] > azumith_max || (tAng[0] - 360) < azumith_min || tAng[1] > elev_max || tAng[1] < elev_min) {
			target = null;
			return;
		}
		
		azumith_current = tAng[0];
		elev_current = tAng[1];
	}
}
