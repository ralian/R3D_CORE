//Originaly created by Zelik
//Use: 
//		MUST HAVE R3D-CORE as dependency.
//		Change your helmet item(whatever you wish to use as sound muffler) from GameEntity to ZEL_PilotHelmet.
//		You will need to use a text editor to do this.  Once changed, your item will now utilize the below functions.
modded class SCR_CharacterInventoryStorageComponent
{    
    // Callback when item is added (will be performed locally after server completed the Insert/Move operation)
    override protected void OnAddedToSlot(IEntity item, int slotID)
    {
        super.OnAddedToSlot(item,slotID);
		#ifdef WORKBENCH
        Print(slotID);
		Print(item);
		Print("Test");
		#endif
        ZEL_PilotHelmet helmet = ZEL_PilotHelmet.Cast(item);
        if(helmet)
            helmet.OnAttach();
        
    }
    
    // Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
    override protected void OnRemovedFromSlot(IEntity item, int slotID)
    {
        super.OnRemovedFromSlot(item,slotID);
        
        ZEL_PilotHelmet helmet = ZEL_PilotHelmet.Cast(item);
        if(helmet)
            helmet.OnDetach();
    }

};

[EntityEditorProps(category: "ZEL", description: "PilotHelmet xD")]
class ZEL_PilotHelmetClass : GameEntityClass{};

class ZEL_PilotHelmet : GameEntity
{
	static IEntity activeHelmet = null;

    float m_OriginalVolume; //Set players original game master volume value
    InventoryItemComponent m_InventoryItemComponent;

	//When attaching helmet to player, set the "Game" master volume to 0.1 to mute game audio
    void OnAttach()
    {
		if(GetGame().GetPlayerController() && GetParent() == GetGame().GetPlayerController().GetControlledEntity()){
        	m_OriginalVolume = AudioSystem.GetMasterVolume(0);
			AudioSystem.SetMasterVolume(0,0.1);
			activeHelmet = this;    
		}		
    }
    
    //When helmet removed, return "Game" master volume to the value set in m_OriginalVolume
    void OnDetach()
    {
		if(activeHelmet==this){
			AudioSystem.SetMasterVolume(0,m_OriginalVolume);
			activeHelmet = null;
		}	
    }

    //Set EventMask, Flags, and m_OriginalVolume
    void ZEL_PilotHelmet(IEntitySource src, IEntity parent)
    {
        SetEventMask(EntityEvent.INIT);
        SetFlags(EntityFlags.ACTIVE, true);
    }

    //Cleanup when session ended and reset "Game" master volume to the original value set in m_OriginalVolume
    void ~ZEL_PilotHelmet()
    {
		if(activeHelmet==this){
			AudioSystem.SetMasterVolume(0,m_OriginalVolume);
			activeHelmet = null;
		}	
    }

};