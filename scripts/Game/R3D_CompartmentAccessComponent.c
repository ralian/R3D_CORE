modded class SCR_CompartmentAccessComponent {
	override protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		auto controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(controller.GetLocalControlledEntity());
		if (!player)
			return;
		
		if (player.m_HeliViewLock) {
			CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent)).SetDisableViewControls(false);
			player.m_HeliViewLock = false;
		}
	}
};
