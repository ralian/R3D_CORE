class R3D_DynamicMassPoint
{
	float mass;
	vector position;
	
	void R3D_DynamicMassPoint(float m, vector p)
	{
		mass = m;
		position = p;
	}
}

class R3D_DynamicMassComponentClass : ScriptComponentClass {}
class R3D_DynamicMassComponent : ScriptComponent 
{
	private IEntity m_Owner;
	private Physics m_Physics;
	private RplComponent m_RplComponent;
	
	private float m_fInitialMass;
	private vector m_vInitialCenterOfMass;
	//private vector m_vInitialInertiaTensor[4];
	
	private ref array<ref R3D_DynamicMassPoint> m_DynamicMassPoints = {};
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SetEventMask(owner, EntityEvent.SIMULATE);
		
		m_Owner = owner;
		m_Physics = owner.GetPhysics();
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (!m_Physics) return;
		m_fInitialMass = m_Physics.GetMass();
		m_vInitialCenterOfMass = m_Physics.GetCenterOfMass();
		//m_Physics.GetInertiaTensor(m_vInitialInertiaTensor);
	}
	
	// Go back to initial state
	void ResetInitialMassProperties()
	{
		m_DynamicMassPoints.Clear();
		m_Physics.SetMass(m_fInitialMass);
		
		// TODO: reset center of mass when Physics::SetCenterOfMass() exists
		//m_Physics.SetCenterOfMass(m_vInitialCenterOfMass);
		
		// TODO: reset interia tensor when Physics::GetInertiaTensor() exists
		//m_Physics.SetInertiaTensorV(m_vInitialInertiaTensor);
	}
	
	static void GetAllChildren(notnull IEntity parent, notnull out array<IEntity> entityArray, bool recursive = false)
    {
        IEntity child = parent.GetChildren();
        while (child)
        {
            entityArray.Insert(child);
            if (recursive)
                GetAllChildren(child, entityArray, true);
            
            child = child.GetSibling();
        }
    }
	
	void UpdateRecursiveMassProperties()
	{
		// If this object has any parent with R3D_DynamicMassComponent then we should reset the mass properties for this object and tell the top-most parent to update its mass properties
		ResetInitialMassProperties();
		
		bool parentWithDynamicMass = false;
		IEntity parent = m_Owner.GetParent();
		while (parent)
		{
			R3D_DynamicMassComponent dynamicMass = R3D_DynamicMassComponent.Cast(parent.FindComponent(R3D_DynamicMassComponent));
			if (dynamicMass)
			{
				parentWithDynamicMass = true;
				dynamicMass.UpdateRecursiveMassProperties();
				break;
			}
			parent = parent.GetParent();
		}
		
		if (parentWithDynamicMass)
			return;
		
		// Iterate children and siblings recursively to account for all possible mass
		ref array<IEntity> children = {};
		GetAllChildren(m_Owner, children, true);
		
		foreach (IEntity child : children)
		{
			Physics childPhysics = child.GetPhysics();
			if (childPhysics)
			{
				float mass = childPhysics.GetMass();
				vector localPos = m_Owner.CoordToLocal(child.GetOrigin());
				
				m_DynamicMassPoints.Insert(new R3D_DynamicMassPoint(mass, localPos));
			}
		}
		
		float additionalMass = 0;
		foreach (R3D_DynamicMassPoint dynamicMassPoint : m_DynamicMassPoints)
		{
			additionalMass += dynamicMassPoint.mass;
		}
		
		m_Physics.SetMass(m_fInitialMass + additionalMass);
	}
	
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_Physics.IsActive() || !m_RplComponent.IsOwner())
			return;
		
		// Add torque to body to account for mass points
		foreach (R3D_DynamicMassPoint massPoint : m_DynamicMassPoints)
		{
			vector worldPos = owner.CoordToParent(massPoint.position);
			vector force = massPoint.mass * Physics.VGravity;
			m_Physics.ApplyImpulseAt(worldPos, force * timeSlice);
		}
	}
}