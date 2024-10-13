class ADM_RigidbodyComponentClass: ScriptComponentClass {}
class ADM_RigidbodyComponent: ScriptComponent
{
	[Attribute(params: "1 inf", defvalue: "1", desc: "Number of substeps per physics frame")]
	protected int m_iSubsteps;
	
	[Attribute(params: "1 inf", defvalue: "1 1 1", desc: "kg*m^2")]
	protected vector m_vInertiaDiag; // [kg*m^2]
	
	// physics stuff
	protected float m_fMass; // [kg]
	protected vector m_vInertia[3]; // [kg*m^2]
	protected vector m_vInertiaInv[3]; // 1/[kg*m^2]
	protected Physics m_Physics;
	
	override event protected void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.SIMULATE);
		
		if (owner.GetPhysics())
		{
			m_Physics = owner.GetPhysics();
			m_fMass = m_Physics.GetMass();
		}
		
		Math3D.MatrixIdentity3(m_vInertia);
		m_vInertia[0][0] = m_vInertiaDiag[0];
		m_vInertia[1][1] = m_vInertiaDiag[1];
		m_vInertia[2][2] = m_vInertiaDiag[2];
		
		Math3D.MatrixIdentity3(m_vInertiaInv);
		m_vInertiaInv[0][0] = 1/m_vInertiaDiag[0];
		m_vInertiaInv[1][1] = 1/m_vInertiaDiag[1];
		m_vInertiaInv[2][2] = 1/m_vInertiaDiag[2];
		
		// add checks here for rigidbody, disable collision and any normal behavior since this is essentially custom rigidbody class
		// just using base rigidbody for fixed simulation loop and define some properties
	}
	
	protected vector forces,moments = vector.Zero;
	void UpdateForcesAndMoments()
	{
		forces = vector.Zero;
		moments = vector.Zero;
	} 
	
	protected vector transform[4];
	protected vector Q[3];
	protected vector COM = vector.Zero;
	protected vector v,w,a,angles = vector.Zero;
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
			
			angles = Math3D.MatrixToAngles(Q);
			
			// ----------------- [ step 2: forces/moments] ------------------
			UpdateForcesAndMoments();
			vector aNext = forces / m_fMass;
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
	
}