class R3D_NEWMissileGuidanceComponentClass: ScriptComponentClass {}
class R3D_NEWMissileGuidanceComponent: ScriptComponent
{
	[Attribute(params: "1 inf", defvalue: "1")]
	protected int m_iSubsteps;
	
	// spring stuff
	[Attribute(params: "1 inf", defvalue: "10", desc: "N/m")]
	protected float m_fStiffness;
	
	[Attribute(params: "-100 100", defvalue: "1", desc: "m")]
	protected float m_fInitialDisplacement;
	
	[Attribute(params: "1 inf", defvalue: "10", desc: "N*m/rad")]
	protected float m_fRotationalStiffness;
	
	[Attribute(params: "1 inf", defvalue: "1 1 1", desc: "kg*m^2")]
	protected vector m_vInertiaDiag; // [kg*m^2]
	
	// physics stuff
	protected float m_fNeutralXposition; // [m]
	protected float m_fMass; // [kg]
	protected vector m_vInertia[3]; // [kg*m^2]
	protected vector m_vInertiaInv[3]; // 1/[kg*m^2]
	protected Physics m_Physics;
	
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.SIMULATE | EntityEvent.FRAME);
		
		if (owner.GetPhysics())
		{
			m_Physics = owner.GetPhysics();
			m_fMass = m_Physics.GetMass();
		}
		
		if (!m_Physics)
			return;
		
		Math3D.MatrixIdentity3(m_vInertia);
		m_vInertia[0][0] = m_vInertiaDiag[0];
		m_vInertia[1][1] = m_vInertiaDiag[1];
		m_vInertia[2][2] = m_vInertiaDiag[2];
		
		Math3D.MatrixIdentity3(m_vInertiaInv);
		m_vInertiaInv[0][0] = 1/m_vInertiaDiag[0];
		m_vInertiaInv[1][1] = 1/m_vInertiaDiag[1];
		m_vInertiaInv[2][2] = 1/m_vInertiaDiag[2];
		
		// initial conditions
		COM = owner.CoordToParent(m_Physics.GetCenterOfMass());
		m_fNeutralXposition = COM[0] - m_fInitialDisplacement;
	}
	
	private vector transform[4];
	private vector Q[3];
	private vector COM = vector.Zero;
	private vector v,w,a = vector.Zero;
	private float rotationalDisplacement,displacement = 0.0;
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_Physics)
			return;
		
		// get current frame state
		owner.GetTransform(transform);
		Q[0] = transform[0];
		Q[1] = transform[1];
		Q[2] = transform[2];
		COM = owner.CoordToParent(m_Physics.GetCenterOfMass());
		
		v = m_Physics.GetVelocity();
		w = m_Physics.GetAngularVelocity();
		
		// Start simple with spring
		float dt = timeSlice/m_iSubsteps;
		for (int i=0; i<m_iSubsteps; i++)
		{
			// Verlet Velocity Integration at time t (conserve energy, euler method leaks energy)
			// 1. calculate changes in position; t + dt
			// 2. find acceleration at t + dt
			// 3. calculate velocity at t + dt
			
			// ----------------- [ step 1] ------------------
			COM += v*dt + 0.5*a*dt*dt;
			
			vector wx[3];
			wx[0][0] =     0;
			wx[0][1] = -w[2]; // -w3
			wx[0][2] =  w[1]; // -w2
			
			wx[1][0] =  w[2]; //  w3
			wx[1][1] =     0;
			wx[1][2] = -w[0]; // -w1

			wx[2][0] = -w[1]; // -w2
			wx[2][1] =  w[0]; //  w1
			wx[2][2] =     0;
			
			vector Qdot[3];
			Math3D.MatrixMultiply3(Q,wx,Qdot);
			
			vector vec0 = Q[0] + Qdot[0]*dt;
			vector vec1 = Q[1] + Qdot[1]*dt;
			vector vec2 = Q[2] + Qdot[2]*dt;
			
			Q[0] = vec0.Normalized();
			Q[1] = vec1.Normalized();
			Q[2] = vec2.Normalized();	
			
			vector angles = Math3D.MatrixToAngles(Q);
			
			// ----------------- [ step 2: forces] ------------------
			
			vector forces = vector.Zero; // [N]
			
			// spring force
			displacement = COM[0] - m_fNeutralXposition;
			forces += -m_fStiffness*displacement *vector.Right;
			
			// get t+dt acceleration			
			vector aNext = forces / m_fMass;
			
			// ----------------- [ step 2: moments ] ------------------
			vector moments = vector.Zero; // [N*m]
			
			// spring force
			rotationalDisplacement = angles[1];
			moments += -m_fRotationalStiffness*rotationalDisplacement *vector.Right;
			
			// get t+dt angular acceleration
			vector part1,alpha;
			Math3D.MatrixMultiply3(m_vInertia,w,part1);
			Math3D.MatrixMultiply3(m_vInertiaInv, moments - w*part1, alpha);
			
			// ----------------- [ step 3 ] ------------------
			v += (a+aNext)*0.5*dt;
			a = aNext;
			
			// TODO: implement verlet, can't find formulation that works.
			w += alpha*dt;
			
		}
		
		// update state from substepping
		transform[0] = Q[0];
		transform[1] = Q[1];
		transform[2] = Q[2];
		transform[3] = COM;
		
		m_Physics.SetVelocity(v);
		m_Physics.SetAngularVelocity(w);
		owner.SetTransform(transform);
		
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		#ifdef WORKBENCH
		DbgUI.Begin("Direct Guidance", 0, 0);
		
		DbgUI.Text(string.Format("w: %1", w.Length()));
		DbgUI.PlotLive("w", 500, 200, w.Length(), timeSlice, 1000);
		
		DbgUI.Text(string.Format("displacement: %1", displacement));
		DbgUI.PlotLive("displacement", 500, 200, displacement, timeSlice, 1000);
		
		DbgUI.Text(string.Format("rotationalDisplacement: %1", rotationalDisplacement));
		DbgUI.PlotLive("rotationalDisplacement", 500, 200, rotationalDisplacement, timeSlice, 1000);
		
		DbgUI.End();
		#endif
	}
	
}