modded class SCR_ChimeraCharacter {
	bool m_HeliViewLock = false;
	
	float m_vehFireLast = 0;
	
	override void EOnInit(IEntity owner) {
		super.EOnInit(owner);
		//GetGame().GetInputManager().AddActionListener("VehicleFire", EActionTrigger.VALUE, VehicleFireCallback);
	}
	
	void VehicleFireCallback(float value, EActionTrigger reason ) {
		if (value > 0.4 && m_vehFireLast < 0.4 && !GetGame().GetInputManager().IsUsingMouseAndKeyboard()) GetGame().GetInputManager().SetActionValue("VehicleFire", 1);
		
		m_vehFireLast = value;
	}
};
