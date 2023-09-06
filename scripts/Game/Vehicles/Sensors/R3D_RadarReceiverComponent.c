enum R3D_RadarSignalType {
	NONE,
	SEARCH,
	TRACK,
	JAM
};

class R3D_RadarTrackingInfo {
	IEntity target = null;
	float seenTime = Replication.Time();
	vector seenLocation = target.GetOrigin();
}

class R3D_RadarReceiverComponentClass : ScriptComponentClass
{
}

class R3D_RadarReceiverComponent: ScriptComponent
{
	[Attribute(desc: "Receiver position in local space")]
	vector origin;
	
	[Attribute(desc: "Effective antenna area (m^2)", defvalue: "1")]
	float A;
	
	// TODO: are the below factors necessary?
	
	[Attribute(desc: "Isotropic factor. All factors should sum to 1", defvalue: "0.1")]
	float factor_iso;
	
	[Attribute(desc: "Parabolic factor. All factors should sum to 1", defvalue: "0.9")]
	float factor_parabolic;
	
	[Attribute(desc: "Isotropic beamwidth (deg)", defvalue: "1.5")]
	float parabolic_beamwidth;
	
	static ref array<IEntity> lsRCVR = {};
	
	ref array<R3D_RadarTrackingInfo> tracks = {};
	
	event void OnDetect(vector posWS, float SNR, R3D_RadarSignalType type) {}
	
	override event protected void EOnInit(IEntity owner) {
		if (Replication.IsServer()) {
			R3D_RadarReceiverComponent.lsRCVR.Insert(owner);
		}
	}
}
