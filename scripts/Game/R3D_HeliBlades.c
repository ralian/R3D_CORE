class R3D_HeliBladesClass : VehicleClass {
	
};

class R3D_HeliBlades : Vehicle {
	
	[Attribute()]
	ResourceName m_ThrownPrefab;
	
	[Attribute("4")]
	int m_NumBlades;
	
	[Attribute("30.0")]
	float m_throwVel;
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void R3D_BreakBlades() {
		//auto owner = GetParent();
		
		//EntitySlotInfo.GetSlotInfo(this);
		//Deactivate();
		delete this;
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact) {
		if (owner.FindComponent(TouchComponent)) {
			if (owner.GetParent() == other)
				return;
			
			if (owner && owner.GetParent() && owner.GetParent().FindComponent(R3D_HeliComponent) && VehicleHelicopterSimulation.Cast(owner.GetParent().FindComponent(VehicleHelicopterSimulation)).EngineIsOn()) {
				array<EntitySlotInfo> outActions = new array<EntitySlotInfo>;
				SlotManagerComponent slots = SlotManagerComponent.Cast(owner.GetParent().FindComponent(SlotManagerComponent));
				slots.GetSlotInfos(outActions);
				
				#ifdef WORKBENCH
				int contactCol = 0xAAFF0000;
				R3D_VehicleSimulationComponent.Cast(owner.GetParent().FindComponent(R3D_VehicleSimulationComponent)).Debug_Add(Shape.CreateSphere(contactCol, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, contact.Position, 0.5));
				#endif
				
				vector bmin, bmax;
				GetWorldBounds(bmin, bmax);
				bmin -= contact.Position;
				bmax -= contact.Position;
				
				//if (vector.Dot(bmin, bmax) > 0) // If the vectors aren't in the same direction, the collision was outside the BB
				//	return;
				
				#ifdef WORKBENCH
				Print("Collison: ");
				SurfaceProperties m1 = contact.Material1;
				Print(owner);
				Print(m1);
				SurfaceProperties m2 = contact.Material2;
				Print(other);
				Print(m2);
				Print(contact.Impulse);
				#endif
				
				/*if (ChimeraCharacter.Cast(other)) {
					other.GetPhysics().SetVelocity("10 0 0"); // fixed direction for now
					ChimeraCharacter.Cast(other).GetDamageManager().Kill();
				}
				
				if (!( StaticModelEntity.Cast(other) || Vehicle.Cast(other) || GenericTerrainEntity.Cast(other) || Tree.Cast(other) || Building.Cast(other) ))
					return;
				
				foreach (EntitySlotInfo item : outActions) {
					if (item.GetAttachedEntity() == this) {
						
						vector rot[3];
						GetParent().GetTransform(rot);
						
						// Throw the blades
						for (int i = 0; i < m_NumBlades; i++) {
							EntitySpawnParams spawnParams = new EntitySpawnParams();
							vector position[4];
							GetWorldTransform(position);
							spawnParams.Transform = position;
							
							IEntity blade = GetGame().SpawnEntityPrefab(Resource.Load(m_ThrownPrefab), null, spawnParams);
							
							blade.GetPhysics().SetActive(ActiveState.ACTIVE);
							vector v = vector.Zero;
							v[0] = m_throwVel * Math.Cos(Math.PI2 * i / m_NumBlades);
							v[2] = m_throwVel * Math.Sin(Math.PI2 * i / m_NumBlades);
							blade.GetPhysics().SetVelocity(rot[0] * v[0] + rot[2] * v[2]);
						}
						
						VehicleHelicopterSimulation.Cast(owner.GetParent().FindComponent(VehicleHelicopterSimulation)).EngineStop();
						VehicleHelicopterSimulation.Cast(owner.GetParent().FindComponent(VehicleHelicopterSimulation)).Deactivate(owner.GetParent());
						owner.GetParent().GetPhysics().EnableGravity(true);
						
						// Hacky angular velocity edit for now. Only changes the azumith velocity, which is wrong, and we need to do a model space transform
						vector angvel = owner.GetParent().GetPhysics().GetAngularVelocity();
						angvel[1] = angvel[1] + Math.PI;
						owner.GetParent().GetPhysics().SetAngularVelocity(angvel);
						
						Rpc(R3D_BreakBlades);
						R3D_BreakBlades();
					}
				}*/
			}
				
				
		}
	}
	
	/*override void EOnPhysicsActive(IEntity owner, bool activeState) {
		SlotManagerComponent sc = SlotManagerComponent.Cast(FindComponent(SlotManagerComponent));
			
		if (sc) {
			array<EntitySlotInfo> acts = new array<EntitySlotInfo>;
				
			EntitySlotInfo.GetSlotInfos(this, acts);
				
			foreach (EntitySlotInfo slot : acts) {
				if (slot.GetAttachedEntity()) {
					string pname = slot.GetAttachedEntity().GetPrefabData().GetPrefabName();
					//Print(pname);
					if (pname == "{630C2AF3AF20FF03}Prefabs/Vehicles/Helis/RAL_UH60/R3D_UH60_Blades.et") {
						if (activeState)
							slot.GetAttachedEntity().GetPhysics().SetActive(ActiveState.ACTIVE); // was ALWAYS_ACTIVE
						else
							slot.GetAttachedEntity().GetPhysics().SetActive(ActiveState.INACTIVE);
					}
				}
			}
		}
	}*/
	
};
