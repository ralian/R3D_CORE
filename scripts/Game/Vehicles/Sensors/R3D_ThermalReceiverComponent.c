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
	
	float azumith_current = 0;
	float elev_current = 0;
	
	R3D_ThermalEmitterComponent target = null;
	
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
