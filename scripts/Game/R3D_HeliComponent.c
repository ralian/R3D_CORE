class R3D_HeliComponentClass : ScriptComponentClass
{
};

class R3D_HeliComponent : ScriptComponent
{
	bool m_Toggled = true;
	bool m_Key = false;
	
	SignalsManagerComponent m_controls_sig;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.SIMULATE);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		//Activate(owner);
		//Vehicle.Cast(owner).Activate(owner, true);
		
		m_controls_sig = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		//m_controls_sig.SetSignalValue(m_controls_sig.FindSignal("RotorScale"), 1);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Check if blades are broken
		// this is really just a failsafe for if blades break when we are not the authority
		array<EntitySlotInfo> outActions = new array<EntitySlotInfo>;
		SlotManagerComponent slots = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		slots.GetSlotInfos(outActions);
		
		bool found = false;
		foreach (EntitySlotInfo item : outActions) {
			if (item.GetAttachedEntity()) {
				if (R3D_HeliBlades.Cast(item.GetAttachedEntity())) {
					found = true;
					
					break;
				}
			}
		}
		
		if (!found) {
			VehicleHelicopterSimulation.Cast(owner.FindComponent(VehicleHelicopterSimulation)).EngineStop();
			VehicleHelicopterSimulation.Cast(owner.FindComponent(VehicleHelicopterSimulation)).Deactivate(owner);
			owner.GetPhysics().EnableGravity(true);
			return;
		}
		
		// Check if pilots seat is occupied
		SCR_BaseCompartmentManagerComponent compartments = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		VehicleHelicopterSimulation vehsim = VehicleHelicopterSimulation.Cast(owner.FindComponent(VehicleHelicopterSimulation));

		array<IEntity> occupants = new array<IEntity>();
		compartments.GetOccupantsOfType(occupants, ECompartmentType.Pilot);

		if (occupants.Count() == 0)
			vehsim.EngineStop();
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(SCR_PlayerController.Cast(GetGame().GetPlayerController()).GetLocalControlledEntity());
		if (occupants.Contains(player)) {
			CharacterControllerComponent controller = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
			
			
			if (Debug.KeyState(KeyCode.KC_LMENU) && !m_Key)
			{
				m_Toggled = !m_Toggled;
			}

			m_Key = Debug.KeyState(KeyCode.KC_LMENU);
			
			//if (controller.GetCameraHandlerComponent().GetFocusMode() > 0);
			// OMFG. The above line works but the below does not. How hard is it BI?
			//if (controller.IsFocusMode())
			//	Print("In focus mode");
			
			//if (m_Toggled) {		
			if (GetGame().GetInputManager().IsUsingMouseAndKeyboard() || (GetGame().GetInputManager().GetActionValue("Focus") > 0) || controller.IsTrackIREnabled()) {
				if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard()) {
					GetGame().GetInputManager().SetActionValue("Focus", 0);
					// These sadly won't alter it before it's used in the engine... need other solution
					//GetGame().GetInputManager().SetActionValue("CyclicForward", 0);
					//GetGame().GetInputManager().SetActionValue("CyclicAside", 0);
				}
				controller.SetDisableViewControls(false);
				controller.SetFreeLook(true);
				player.m_HeliViewLock = false;
			} else {
				controller.SetDisableViewControls(true);
				controller.SetFreeLook(false);
				player.m_HeliViewLock = true;
			}
		}
	}
};
