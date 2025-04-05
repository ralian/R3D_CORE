class R3D_ThermalReceiverComponentClass : ScriptComponentClass
{
}

class R3D_ThermalReceiverComponent: ScriptComponent
{
	[Attribute(desc: "Receiver position in local space", uiwidget: UIWidgets.EditBox, params: "inf inf 0 purpose=coords space=entity")]
	vector origin;
	
	[Attribute(desc: "Start azumith of the search volume in deg", defvalue: "-45")]
	float azumith_min;
	
	[Attribute(desc: "End azumith of the search volume in deg", defvalue: "45")]
	float azumith_max;
	
	[Attribute(desc: "Start elev of the search volume in deg", defvalue: "-45")]
	float elev_min;
	
	[Attribute(desc: "End elev of the search volume in deg", defvalue: "45")]
	float elev_max;
	
	[Attribute(desc: "Beam radius in deg", defvalue: "7")]
	float beam_angle;
	
	float azumith_current = 0;
	float elev_current = 0;
	
	R3D_ThermalEmitterComponent target = null;
	
	vector GetSeekerForwardWS() {
		vector tform[4];
		GetOwner().GetWorldTransform(tform);
		vector rotMat[3];
		Math3D.AnglesToMatrix(vector.Up * elev_current + vector.Right * azumith_current, rotMat);
		return GetOwner().CoordToParent(rotMat[2] + origin) - GetOwner().CoordToParent(origin);
	}
	
	override void OnPostInit(IEntity owner) {
		SetEventMask(owner, EntityEvent.SIMULATE);
		#ifdef WORKBENCH
		ConnectToDiagSystem(owner);
		#endif // WORKBENCH
	}
	
	void _DbgVisualize(IEntity owner, float timeSlic) {
		vector tform[4];
		owner.GetWorldTransform(tform);		
		Math3D.MatrixFromForwardVec(GetSeekerForwardWS(), tform);
		vector seekerLS = owner.CoordToParent(origin);
		
		float y_factor = Math.Tan(beam_angle * Math.DEG2RAD);
		vector p[] = {
			seekerLS, seekerLS + tform[2] * 2000,
			seekerLS, seekerLS + 2000 * (tform[2] + y_factor * tform[0]),
			seekerLS, seekerLS + 2000 * (tform[2] - y_factor * tform[0]),
			seekerLS, seekerLS + 2000 * (tform[2] + y_factor * tform[1]),
			seekerLS, seekerLS + 2000 * (tform[2] - y_factor * tform[1])
		};
		Shape.CreateLines(COLOR_GREEN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, p, 10);
	}
	
	ref array<R3D_ThermalEmitterComponent> SeekerTargets = {};
	bool AddTarget(IEntity ent) {
		if (ent != GetOwner().GetParent() && ent.FindComponent(R3D_ThermalEmitterComponent))
			SeekerTargets.Insert(R3D_ThermalEmitterComponent.Cast(ent.FindComponent(R3D_ThermalEmitterComponent)));
		return true;
	}
	
	R3D_ThermalEmitterComponent TryLock() {
		const vector mins = "-100 -100 10", maxs = "100 100 2000"; // This is also a bad assumption obviously
		vector tform[4];
		Math3D.MatrixFromForwardVec(GetSeekerForwardWS(), tform);
		tform[3] = GetOwner().CoordToParent(origin);
		
		SeekerTargets.Clear();
		GetGame().GetWorld().QueryEntitiesByOBB(mins, maxs, tform, AddTarget);
		foreach (R3D_ThermalEmitterComponent te : SeekerTargets) {
			// BAD: This assumes that elev_current and azumith_current are 0. Needs rewritten.
			const vector targetLS = GetOwner().CoordToLocal(te.GetOwner().CoordToParent(te.origin));
			const float xyDist = Math.Sqrt(targetLS[0] * targetLS[0] + targetLS[1] * targetLS[1]);
			if (Math.RAD2DEG * Math.Atan2(xyDist, targetLS[2]) > beam_angle) continue;
			if (te) target = te;
		} 
		return target;
	}
	
	void DropLock() {
		azumith_current = 0;
		elev_current = 0;
		target = null;
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice) {
		if (!target) return;
		
		// Make sure the current target is within the gimbal limits
		vector targetWS = target.GetOwner().CoordToParent(target.origin);
		vector targetLS = owner.CoordToLocal(targetWS) - origin;
		
		vector tAng = targetLS.VectorToAngles();
		
		while (tAng[0] > 180) tAng[0] = tAng[0] - 360;
		while (tAng[1] > 180) tAng[1] = tAng[1] - 360;
		
		if (tAng[0] > azumith_max || tAng[0] < azumith_min || tAng[1] > elev_max || tAng[1] < elev_min) {
			DropLock();
			return;
		}
		
		// Raycast to the target to ensure we have LOS
		vector sensorWS = owner.CoordToParent(origin);
		ref TraceParam param = new TraceParam;
		param.Exclude = owner;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		param.LayerMask = EPhysicsLayerDefs.Camera;
		param.Start = sensorWS;
		param.End = targetWS;
		
		float dist = GetGame().GetWorld().TraceMove(param, null);
		
		if (vector.Distance(sensorWS, targetWS)*(1-dist) > target.radius) {
			DropLock();
			return;
		}
		
		azumith_current = tAng[0];
		elev_current = tAng[1];
	}
	
	override void EOnDiag(IEntity owner, float timeSlice) {
		if (owner && owner.GetParent() && owner.GetParent().FindComponent(ADM_AirplaneInput)) {
			ADM_AirplaneInput input = ADM_AirplaneInput.Cast(owner.GetParent().FindComponent(ADM_AirplaneInput));
			if (input)
				_DbgVisualize(owner, timeSlice);
		}
	}
	
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice) {
		//_DbgVisualize(owner, timeSlice);
	}
}
