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
	
	// ASSUMPTION: center of mass equals origin of model space!
	override event protected void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.SIMULATE | EntityEvent.INIT);
		
		// add checks here for rigidbody, disable collision and any normal behavior since this is essentially custom rigidbody class
		// just using base rigidbody for fixed simulation loop and define some properties
	}
	
	override event protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
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
		
		// Initial State, for some reason if I initialize this in OnPostInit simulation will get NaN in transformation occasionally
		COM = owner.CoordToParent(m_Physics.GetCenterOfMass());
		owner.GetTransform(transform);
		Q[0] = transform[0];
		Q[1] = transform[1];
		Q[2] = transform[2];
		
		Qinv[0][0] = Q[0][0];
		Qinv[0][1] = Q[1][0];
		Qinv[0][2] = Q[2][0];
		
		Qinv[1][0] = Q[0][1];
		Qinv[1][1] = Q[1][1];
		Qinv[1][2] = Q[2][1];
		
		Qinv[2][0] = Q[0][2];
		Qinv[2][1] = Q[1][2];
		Qinv[2][2] = Q[2][2];
	}
	
	protected vector forces,moments = vector.Zero;
	void UpdateForcesAndMoments(IEntity owner, float curTime = System.GetTickCount())
	{
		forces = vector.Zero;
		moments = vector.Zero;
	} 
	
	vector CoordToLocal(vector worldCoord)
	{
		return (worldCoord - COM).Multiply3(Qinv);
	}
	
	vector CoordToWorld(vector localCoord)
	{
		return localCoord.Multiply3(Q) + COM;
	}
	
	vector VectorToLocal(vector worldVector)
	{
		return worldVector.Multiply3(Qinv);
	}
	
	vector VectorToWorld(vector localVector)
	{
		return localVector.Multiply3(Q);
	}
	
	protected vector transform[4];
	protected vector Q[3], Qinv[3];
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
		
		// TODO: replace GetTickCount with proper world time to include pausing/fast forward/etc.
		float substepStartTime = System.GetTickCount();
		float dt = timeSlice/m_iSubsteps;
		for (int i=0; i<m_iSubsteps; i++)
		{
			float curTime = substepStartTime + dt*i;
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
			
			Qinv[0][0] = Q[0][0];
			Qinv[0][1] = Q[1][0];
			Qinv[0][2] = Q[2][0];
			
			Qinv[1][0] = Q[0][1];
			Qinv[1][1] = Q[1][1];
			Qinv[1][2] = Q[2][1];
			
			Qinv[2][0] = Q[0][2];
			Qinv[2][1] = Q[1][2];
			Qinv[2][2] = Q[2][2];
			
			angles = Math3D.MatrixToAngles(Q);
			
			// ----------------- [ step 2: forces/moments] ------------------
			UpdateForcesAndMoments(owner, curTime);
			vector aNext = forces / m_fMass;
			vector alpha = m_vInertiaInv.Multiply3(moments - w*(m_vInertia.Multiply3(w)));
			
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
		m_Physics.SetMass(m_fMass);
		owner.SetTransform(transform);
		
	}
	
}