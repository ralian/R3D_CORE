class R3D_WheelComponentClass : R3D_VehiclePartBaseComponentClass
{
};

class R3D_WheelComponent : R3D_VehiclePartBaseComponent
{
	[Attribute("-1", UIWidgets.ComboBox, "Wheel bone")]
	int m_Bone;

	[Attribute("0.5", UIWidgets.Auto, "The radius of the wheel")]
	float m_Radius;

	[Attribute("0", UIWidgets.ComboBox, "LayerMask", "", ParamEnumArray.FromEnum(EPhysicsLayerDefs))]
	int m_TraceLayer;
	
	[Attribute()]
	float m_Stiffness; //! TODO: spring rate

	[Attribute()]
	float m_CompressionDamper;

	[Attribute()]
	float m_RelaxationDamper;

	[Attribute()]
	float m_MaxTravelUp;
	
	[Attribute()]
	float m_MaxTravelDown;
	
	[Attribute()]
	string m_SignalName;

	[Attribute()]
	int m_ControlIndex;

	int m_SignalIndex;

	[Attribute(defvalue: "1.5 0 0.5", desc: "")]
	vector m_FrictionCoefs;
	
	[Attribute()]
	bool m_Brake;

	vector m_LinePoints[2];

	vector m_InitialTransform[4];

	vector m_Transform[4];

	float m_MaxTravel;
	
	vector m_Trace;
	float m_TraceUp;
	float m_TraceDown;
	float m_ContactLength;

	vector m_RayStartMS;
	vector m_RayStartWS;
	vector m_RayEndMS;
	vector m_RayEndWS;

	float m_ContactFraction;
	bool m_HasContact;

	vector m_ContactPosition;
	vector m_ContactNormal;
	vector m_ContactVelocity;

	float m_SuspensionLength;
	float m_SuspensionFraction;
	vector m_SuspensionForce;

	float m_SuspensionRelativeVelocity;
	float m_SuspensionInvContact;

	vector m_TransformWS[4];
	vector m_ContactPositionWS;
	vector m_ContactNormalWS;
	vector m_SuspensionForceWS;

	string GetBoneNameForIndex(int index)
	{
		IEntity owner = GetOwner();
		array<string> boneNames();
		owner.GetAnimation().GetBoneNames(boneNames);
		foreach (auto name : boneNames)
		{
			if (index == owner.GetAnimation().GetBoneIndex(name))
			{
				return name;
			}
		}
		return string.Empty;
	}

	override bool OnInitialize(R3D_VehicleSimulationComponent simulation)
	{
		if (!super.OnInitialize(simulation))
		{
			return false;
		}
		
		IEntity owner = GetOwner();
		owner.GetAnimation().GetBoneMatrix(m_Bone, m_Transform);

		m_InitialTransform[0] = "-1 0 0";
		m_InitialTransform[1] = "0 -1 0";
		m_InitialTransform[2] = "0 0 -1";
		m_InitialTransform[3] = m_Transform[3];
		
		m_MaxTravel = m_MaxTravelUp + m_MaxTravelDown;

		Print(m_ProcAnim);
		Print(m_ControlIndex);
		Print(m_SignalName);
		Print(m_SignalIndex);

		return true;
	}

	override void OnCompute(float dt, Physics physics)
	{
		IEntity owner = GetOwner();
		BaseWorld world = owner.GetWorld();

		owner.GetAnimation().GetBoneMatrix(m_Bone, m_Transform);
		//! TODO: steering rotation

		m_Simulation.Multiply4(m_Transform, m_TransformWS);

		m_Trace = m_InitialTransform[3];

		m_TraceUp = m_MaxTravelUp;
		m_TraceDown = m_MaxTravelDown + m_Radius;

		m_ContactLength = m_TraceUp + m_TraceDown;
		
		m_RayStartMS = m_Trace - (m_InitialTransform[1] * m_TraceUp);
		m_RayEndMS = m_Trace + (m_InitialTransform[1] * m_TraceDown);
		
		TraceParam trace();
		//trace.Radius = m_Radius;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;// | TraceFlags.OCEAN;
		trace.LayerMask = m_TraceLayer;
		trace.Exclude = owner;

		m_Simulation.VMultiply4(m_RayStartMS, trace.Start);
		m_Simulation.VMultiply4(m_RayEndMS, trace.End);

#ifdef WORKBENCH
		//m_Simulation.Debug_Add(Shape.CreateSphere(0xAAFF0000, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, trace.Start, m_Radius));
		//m_Simulation.Debug_Add(Shape.CreateSphere(0xAAFF0000, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, trace.End, m_Radius));
#endif

		m_ContactFraction = world.TraceMove(trace, null);

		m_HasContact = m_ContactFraction < 1.0;

		if (m_HasContact)
		{
			m_SuspensionLength = m_ContactLength * (1.0 - m_ContactFraction);
			m_SuspensionFraction = m_SuspensionLength / m_MaxTravel;

			m_ContactPositionWS = ((trace.End - trace.Start) * m_ContactFraction) + trace.Start;
			m_ContactNormalWS = trace.TraceNorm;

			m_Simulation.VInvMultiply4(m_ContactPositionWS, m_ContactPosition);
			m_Simulation.VInvMultiply3(m_ContactNormalWS, m_ContactNormal);

			m_ContactVelocity = m_Simulation.GetModelVelocityAt(m_ContactPosition);

			float denominator = vector.Dot(m_ContactNormal, m_InitialTransform[1]);
			if (denominator >= -0.1)
			{
				m_SuspensionRelativeVelocity = 0.0;
				m_SuspensionInvContact = 10.0;
			}
			else
			{
				float inv = -1.0 / denominator;
				m_SuspensionRelativeVelocity = vector.Dot(m_ContactNormal, m_ContactVelocity) * inv;
				m_SuspensionInvContact = inv;
			}

			float compression = m_RelaxationDamper * m_SuspensionRelativeVelocity;
			if (m_SuspensionRelativeVelocity < 0)
			{
				compression = m_CompressionDamper * m_SuspensionRelativeVelocity;
			}

			float suspension = m_SuspensionLength;
			suspension *= m_Stiffness * m_SuspensionInvContact;

			m_SuspensionForce = m_ContactNormal * Math.Max(suspension - compression, 0.0);
		}

		if (!m_HasContact)
		{
			m_SuspensionForce = vector.Zero;
			m_SuspensionForceWS = vector.Zero;

			m_ContactFraction = 1.0;
			m_ContactVelocity = vector.Zero;

			m_SuspensionRelativeVelocity = 0.0;
			m_SuspensionInvContact = 1.0;

			m_SuspensionLength = 0.0;

			m_ContactPosition = m_Trace + (m_InitialTransform[1] * m_TraceDown);
			m_ContactNormal = m_Transform[1];

			m_Simulation.VMultiply4(m_ContactPosition, m_ContactPositionWS);
			
			m_ContactNormalWS = m_TransformWS[1];
		}

#ifdef WORKBENCH
		int contactCol = 0xAAFF0000;
		if (m_HasContact) contactCol = 0xAA0000FF;
		m_Simulation.Debug_Add(Shape.CreateSphere(contactCol, ShapeFlags.TRANSP | ShapeFlags.NOZBUFFER, m_ContactPositionWS, m_Radius));
#endif
	}

	override void OnSimulate(float dt, Physics physics)
	{
		if (!m_HasContact)
		{
			return;
		}

		float mass = physics.GetMass();
		float invMass = 1.0 / physics.GetMass();
		float dtInvMass = invMass / dt;

		vector impulse = vector.Zero;

		// Add suspension force as impulse 
		impulse += m_SuspensionForce * dt;

		vector frictDir = -m_ContactVelocity;
		
		if (m_Brake)
			frictDir[2] = frictDir[2] * 10;
		
		impulse += Vector(m_FrictionCoefs[0] * frictDir[0], /*m_FrictionCoefs[1] * frictDir[1]*/ 0, m_FrictionCoefs[2] * frictDir[2]) * dt * Math.Min(m_SuspensionForce.Length(), mass) / 100;// * m_SuspensionForce.Length() * dt;
		
		// Convert to world space
		m_Simulation.VMultiply3(impulse, impulse);

		// Apply the impulse
		physics.ApplyImpulseAt(m_ContactPositionWS, impulse);
		
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		m_SignalIndex = signalsManagerComponent.FindSignal(m_SignalName);

		if (m_SignalIndex != -1)
		{
			m_SignalsManager.SetSignalValue(m_SignalIndex, m_SuspensionFraction);
		}

#ifdef WORKBENCH
		// Debug
		vector velocity;
		m_Simulation.VMultiply3(m_ContactVelocity, velocity);

		m_LinePoints[0] = m_ContactPositionWS;
		m_LinePoints[1] = m_ContactPositionWS + velocity;
		m_Simulation.Debug_Add(Shape.CreateLines(0xFFFFFF00, ShapeFlags.NOZBUFFER, m_LinePoints, 2));

		//m_LinePoints[0] = m_ContactPositionWS;
		//m_LinePoints[1] = m_ContactPositionWS + (impulse * dtInvMass);
		//m_Simulation.Debug_Add(Shape.CreateLines(0xFFFFFF00, ShapeFlags.NOZBUFFER, m_LinePoints, 2));
#endif
	}

#ifdef WORKBENCH
	override array<ref ParamEnum> _WB_GetUserEnums(string varName, IEntity owner, IEntityComponentSource src)
	{
		if (varName == "m_Bone")
		{
			return SCR_Global.GetBonesAsParamEnums(owner);
		}

		return super._WB_GetUserEnums(varName, owner, src);
	}
#endif
};
