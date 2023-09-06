class R3D_VehicleSimulationComponentClass : ScriptComponentClass
{
};

class R3D_VehicleSimulationComponent : ScriptComponent
{
	private ref array<R3D_VehiclePartBaseComponent> m_Parts;
	private ref array<R3D_WheelComponent> m_Wheels;

	private vector m_LinearVelocityMS;
	private vector m_AngularVelocityMS;
	
	float m_ControlRoll;
	float m_ControlPitch;
	float m_ControlYaw;
	float m_ControlBrake;
	float m_ControlParkBrake;
	float m_DebugSimulateAerofoils;
	float m_ControlThrust;
	vector m_LinearVelocity;
	vector m_AngularVelocity;
	
	float GetNumberWheels(){
		return 3.0;
	}
	
	float GetDensity(){
		return 1.0;
	}

	private vector m_Transform[4];

	void R3D_VehicleSimulationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Parts = new array<R3D_VehiclePartBaseComponent>();
		m_Wheels = new array<R3D_WheelComponent>();
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT | EntityEvent.SIMULATE);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}

	override void EOnInit(IEntity owner)
	{
		UpdateComponents(R3D_WheelComponent);
	}

	private void UpdateComponents(typename type)
	{
		array<Managed> components();
		GetOwner().FindComponents(type, components);
		foreach (auto component : components)
		{
			auto part = R3D_VehiclePartBaseComponent.Cast(component);
			if (!part.OnInitialize(this))
			{
				continue;
			}

			m_Parts.Insert(part);

			auto wheel = R3D_WheelComponent.Cast(part);
			if (wheel)
			{
				m_Wheels.Insert(wheel);
			}
		}
	}

	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		Debug_Clear();

		Physics physics = owner.GetPhysics();

		if (!physics)
		{
			return;
		}

		if (!physics.IsActive() || !physics.IsDynamic())
		{
			return;
		}

		owner.GetTransform(m_Transform);

		VInvMultiply3(physics.GetVelocity(), m_LinearVelocityMS);
		VInvMultiply3(physics.GetAngularVelocity(), m_AngularVelocityMS);
		
		foreach (auto part : m_Parts)
		{
			part.OnCompute(timeSlice, physics);
		}

		foreach (auto part : m_Parts)
		{
			part.OnSimulate(timeSlice, physics);
		}
	}

	vector GetModelVelocityAt(vector relPos)
	{
		return m_LinearVelocityMS + (m_AngularVelocityMS * relPos);
	}

	vector GetWorldVelocityAt(vector relPos)
	{
		return (m_LinearVelocityMS + (m_AngularVelocityMS * relPos)).Multiply3(m_Transform);
	}

	void Multiply4(vector inTransform[4], inout vector outTransform[4])
	{
		Math3D.MatrixMultiply4(m_Transform, inTransform, outTransform);
	}

	void InvMultiply4(vector inTransform[4], inout vector outTransform[4])
	{
		Math3D.MatrixInvMultiply4(m_Transform, inTransform, outTransform);
	}

	void Multiply3(vector inTransform[4], inout vector outTransform[4])
	{
		Math3D.MatrixMultiply3(m_Transform, inTransform, outTransform);
	}

	void InvMultiply3(vector inTransform[4], inout vector outTransform[4])
	{
		Math3D.MatrixInvMultiply3(m_Transform, inTransform, outTransform);
	}

	void VMultiply4(vector inVector, inout vector outVector)
	{
		outVector = inVector.Multiply4(m_Transform);
	}

	void VInvMultiply4(vector inVector, inout vector outVector)
	{
		outVector = inVector.InvMultiply4(m_Transform);
	}

	void VMultiply3(vector inVector, inout vector outVector)
	{
		outVector = inVector.Multiply3(m_Transform);
	}

	void VInvMultiply3(vector inVector, inout vector outVector)
	{
		outVector = inVector.InvMultiply3(m_Transform);
	}

	ref array<Shape> m_DebugShapes = new array<Shape>();
	ref array<DebugText> m_DebugText = new array<DebugText>();
	ref array<R3D_DebugShape> m_DebugR3DShapes = new array<R3D_DebugShape>();

	void Debug_Add(Shape shape)
	{
		m_DebugShapes.Insert(shape);
	}
	void Debug_Add(R3D_DebugShape shape)
	{
		m_DebugR3DShapes.Insert(shape);
	}

	void Debug_Add(DebugText text)
	{
		m_DebugText.Insert(text);
	} 

	void Debug_Clear()
	{
		foreach (auto shape : m_DebugShapes)
		{
			shape = null;
		}

		m_DebugShapes.Clear();

		foreach (auto text : m_DebugText)
		{
			text = null;
		}

		m_DebugText.Clear();
	}
};
