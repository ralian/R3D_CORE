class R3D_RadarEmitterComponentClass : ScriptComponentClass
{
}

class R3D_RadarEmitterComponent: ScriptComponent
{
	bool power_on = false;
	
	[Attribute(desc: "Emitter position in local space")]
	vector origin;
	
	[Attribute(desc: "Start azumith of the search volume in deg", defvalue: "-90")]
	float azumith_min;
	
	[Attribute(desc: "End azumith of the search volume in deg", defvalue: "90")]
	float azumith_max;
	
	[Attribute(desc: "Start elev of the search volume in deg", defvalue: "-20")]
	float elev_min;
	
	[Attribute(desc: "End elev of the search volume in deg", defvalue: "20")]
	float elev_max;
	
	[Attribute(desc: "Radar transmitter power (W)", defvalue: "150")]
	float P_t;
	
	[Attribute(desc: "Radar bandwidth (Hz)", defvalue: "1000000")]
	float B_n;
	
	[Attribute(desc: "System noise temp (K)", defvalue: "900")]
	float T_s;
	
	[Attribute(desc: "Wavelength (m)", defvalue: "0.1")]
	float lambda;
	
	[Attribute(desc: "Beamwidth azumith (deg)", defvalue: "1.5")]
	float beamwidth_azumith;
	
	[Attribute(desc: "Search volume frequency (Hz)", defvalue: "1")]
	float scan_period;
	
	float time_since_last_scan = 0.0;
	
	[Attribute(desc: "Total system losses. Convert from dB, 8dB is a good estimate", defvalue: "6.3")]
	float L;
	
	// If we have a receiver in the same vehicle as the transmitter, set it here in EOnInit to make some math easier
	R3D_RadarReceiverComponent associated_antenna = null;
	
	// Signal to Noise Ratio assuming direct LOS when tracking target
	float get_snr_tracking(R3D_RadarCrossSectionComponent target, R3D_RadarReceiverComponent receiver) {
		// Current emitter position
		vector emmitterPosWS = GetOwner().CoordToParent(origin);
		
		// Gain
		float G = 4 * Math.PI * receiver.A / (lambda * lambda);
		
		// Effective RCS area
		float sigma = target.GetEffectiveRCSFrom(emmitterPosWS);
		
		// R1 is distance from emmitter to target, R2 is distance from target to receiver 
		float R1sq = vector.DistanceSq(emmitterPosWS, target.GetOwner().GetOrigin());
		float R2sq = vector.DistanceSq(target.GetOwner().GetOrigin(), receiver.GetOwner().CoordToParent(receiver.origin));
		
		return P_t * G * G * lambda * lambda * sigma / (64 * R3D_Math.PIPow3 * R1sq * R2sq * R3D_Math.Kb * T_s * B_n * L);
	}
	
	// Signal to Noise Ratio assuming direct LOS when target is in the search volume
	float get_snr_searching(R3D_RadarCrossSectionComponent target, R3D_RadarReceiverComponent receiver) {
		// Current emitter position
		vector emmitterPosWS = GetOwner().CoordToParent(origin);
		
		// Effective RCS area
		float sigma = target.GetEffectiveRCSFrom(emmitterPosWS);
		
		// R1 is distance from emmitter to target, R2 is distance from target to receiver 
		float R1sq = vector.DistanceSq(emmitterPosWS, target.GetOwner().GetOrigin());
		float R2sq = vector.DistanceSq(target.GetOwner().GetOrigin(), receiver.GetOwner().CoordToParent(receiver.origin));
		
		return P_t * receiver.A * sigma / (4 * Math.PI * (azumith_max - azumith_min) * Math.DEG2RAD * scan_period * R1sq * R2sq * R3D_Math.Kb * T_s * L);
	}
	
	// Todo this will do the vector math of if the RCS is outside the search volume, return 0.0
	// otherwise return the result of a raycast and fudge it a bit for some of the collision layers
	float has_line_of_sight(R3D_RadarCrossSectionComponent target, R3D_RadarReceiverComponent receiver) {
		return 1.0;
	}
	
	override event protected void EOnSimulate(IEntity owner, float timeSlice) {
		if (!Replication.IsServer() || !power_on) return;
		
		time_since_last_scan += timeSlice;
		
		if (time_since_last_scan > scan_period)
			time_since_last_scan = 0.0;
		else
			return;
		
		foreach (IEntity veh : R3D_RadarCrossSectionComponent.lsRCS) {
			if (!veh) continue;
			
			R3D_RadarCrossSectionComponent rcs = R3D_RadarCrossSectionComponent.Cast(veh.FindComponent(R3D_RadarCrossSectionComponent));
			if (!rcs) continue;
		
			foreach (IEntity vehRcvr : R3D_RadarReceiverComponent.lsRCVR) {
				if (!vehRcvr) continue;
			
				R3D_RadarReceiverComponent rcvr = R3D_RadarReceiverComponent.Cast(vehRcvr.FindComponent(R3D_RadarReceiverComponent));
				if (!rcs) continue;
				
				float LOS = has_line_of_sight(rcs, rcvr);
				if (LOS > 0.01) {
					rcvr.OnDetect(veh.GetOrigin(),
						get_snr_searching(rcs, rcvr) * LOS,
						R3D_RadarSignalType.SEARCH
						);
				}
			}
		}
	}
}