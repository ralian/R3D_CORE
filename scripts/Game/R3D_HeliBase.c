class R3D_HeliBaseClass : VehicleClass {
	
};

class R3D_HeliBase : Vehicle {
	[Attribute(defvalue: "12.0", desc: "Maximum amount the rotor blades may warp. Dependent on weight painting, not a distance.")]
	float MAX_ROTOR_WARP;
	
	float m_LastRPM = -1;
	
	[Attribute(defvalue: "0.75", desc: "Amount the blades warp per unit of acceleration on them.")]
    float m_NominalRotorWarp;

    float m_RotAngPri = 0;
    float m_RotAngSec = 0;

    float m_LastLocalClimbRate = 0;
	
    int m_InteriorEmissiveLevel = 0;
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)] //Marks to engine that this is Replication function
	void R3D_SetBladeBroken() {
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		signalsManagerComponent.SetSignalValue(signalsManagerComponent.FindSignal("EngineBroken"), 1);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice) {
		super.EOnFrame(owner, timeSlice);
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		VehicleHelicopterSimulation vehsim = VehicleHelicopterSimulation.Cast(FindComponent(VehicleHelicopterSimulation));
		ParametricMaterialInstanceComponent panelLights = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));
		
		if (panelLights)
			panelLights.SetEmissiveMultiplier(m_InteriorEmissiveLevel);
		
		//Set custom signals RotorRPMprimary-RotorAngPriIn-RotorAngSecIn
		if (signalsManagerComponent) { 
			int RotorRPMprimary = signalsManagerComponent.FindSignal("RotorRPMprimary");
			int RotorAngPriIn = signalsManagerComponent.FindSignal("RotorAngPriIn");
			int RotorAngSecIn = signalsManagerComponent.FindSignal("RotorAngSecIn");
			int RotorWarp = signalsManagerComponent.FindSignal("RotorWarp");
			
			vector tmat[4];
            GetTransform(tmat);
            float localClimbRate = vector.Dot(GetPhysics().GetVelocity(), tmat[1]);
			float curRPM = signalsManagerComponent.GetSignalValue(RotorRPMprimary);
			if (curRPM > 0.1 && m_LastRPM <= 0.1)
			{				
				MotorExhaustEffectComponent exhaust = MotorExhaustEffectComponent.Cast(owner.FindComponent(MotorExhaustEffectComponent));
				if (exhaust)
					exhaust.TurnOn(owner);
			}
			if (vehsim.IsActive()) {
				signalsManagerComponent.SetSignalValue(RotorWarp, Math.Clamp(m_NominalRotorWarp * (signalsManagerComponent.GetSignalValue(RotorRPMprimary) / 100.0) + (localClimbRate - m_LastLocalClimbRate) * 0.5 / timeSlice, -MAX_ROTOR_WARP, MAX_ROTOR_WARP));
				
				m_RotAngPri += 2.0 * (curRPM * timeSlice); //add 2 times (current RotorRPMprimary value times timeSlice) to current m_RotAngPri value.
				m_RotAngPri = Math.Repeat(m_RotAngPri, 360.0); // Clamp m_RotAngPri to 0-360
				signalsManagerComponent.SetSignalValue(RotorAngPriIn, 360 - m_RotAngPri);
				signalsManagerComponent.SetSignalValue(RotorAngSecIn, m_RotAngPri);
				
				vector euler = GetYawPitchRoll();
				signalsManagerComponent.SetSignalValue(signalsManagerComponent.FindSignal("pitch"), euler[1]); //Set signal for "pitch" and "roll" via GetLocalYawPitchRoll.  Yaw-0 Pitch-1 Roll-2
				signalsManagerComponent.SetSignalValue(signalsManagerComponent.FindSignal("roll"), euler[2]);
				signalsManagerComponent.SetSignalValue(signalsManagerComponent.FindSignal("heading"), euler[0]);
			} else {

				if (signalsManagerComponent.GetSignalValue(RotorRPMprimary) > 0) {
					//SCR_MotorExhaustEffectGeneralComponent exhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(owner.FindComponent(SCR_MotorExhaustEffectGeneralComponent));
					signalsManagerComponent.SetSignalValue(signalsManagerComponent.FindSignal("EngineON"), 0); //Sets EngineOn if RotorRPMprimary is greater than 0
					Rpc(R3D_SetBladeBroken);
					R3D_SetBladeBroken();
				}
			}
			
			m_LastLocalClimbRate = localClimbRate;
			m_LastRPM = curRPM;
		}
	}
	
	// Until we fix the collision check against terrain issue, we should refrain from activating the blades. They can still be activated by 
	// the engine in certain situations, but as long as the blades don't have an associated rigidbody component they should be fine. Creating 
	// the Ghost collider via script would produce the same behavior, sadly.
	
	/*override void EOnPhysicsActive(IEntity owner, bool activeState) {
       SlotManagerComponent sc = SlotManagerComponent.Cast(FindComponent(SlotManagerComponent));

        if (sc) {
            array<EntitySlotInfo> acts = new array<EntitySlotInfo>;
            EntitySlotInfo.GetSlotInfos(this, acts);
            foreach (EntitySlotInfo slot : acts) {
                if (slot.GetAttachedEntity() && R3D_HeliBlades.Cast(slot.GetAttachedEntity())) {
					if (activeState) {
						PhysicsGeomDef geoms[] = {PhysicsGeomDef("", PhysicsGeom.CreateCylinder(5.0, 0.2), "{CE9253778DD8FBDE}Common/Materials/Game/metal.gamemat", EPhysicsLayerDefs.Vehicle)};
						Physics.CreateGhostEx(slot.GetAttachedEntity(), geoms);
						slot.GetAttachedEntity().GetPhysics().SetActive(ActiveState.ALWAYS_ACTIVE);
					} else {
						slot.GetAttachedEntity().GetPhysics().SetActive(ActiveState.INACTIVE);
					}
                }
            }
        }
    }*/
};
