class NwkPlaneMovementComponentClass: ScriptGameComponentClass {}
class NwkPlaneMovementComponent : ScriptGameComponent
{
	protected IEntity m_Owner;
	protected Physics m_Physics;
	protected RplComponent m_RplComponent;
	
	// interpolation
	[Attribute(defvalue: "1")]
	protected bool m_bInterpolate;
	
	protected vector prevState[4];
	protected vector prevVelocity;
	protected vector prevAngularVelocity;
	protected float prevStateTime;
	
	protected vector curState[4];
	protected vector curVelocity;
	protected vector curAngularVelocity;
	protected float curStateTime;
	
	override void OnPostInit(IEntity owner)
	{
		m_Owner = owner;
		m_Physics = owner.GetPhysics();
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		m_Owner.GetTransform(prevState);
		m_Owner.GetTransform(curState);
		prevStateTime = System.GetTickCount();
		curStateTime = System.GetTickCount();
		
		SetEventMask(owner, EntityEvent.POSTFRAME);	
	}
	
	override event protected bool OnTicksOnRemoteProxy() { return true; };
	
	private float m_fLastSendTime = -float.MAX;
	private float m_fUpdateDelay = 50;
	override event protected void EOnPostFrame(IEntity owner, float timeSlice)
	{
		super.EOnPostFrame(owner, timeSlice);
		
		float curTime = System.GetTickCount();
		if (m_RplComponent.IsOwner())
		{
			float dtSend = curTime - m_fLastSendTime;
			if (dtSend > m_fUpdateDelay)
			{
				vector mat[4];
				owner.GetTransform(mat);
				Rpc(Rpc_Server_ReceiveNewStates, mat, m_Physics.GetVelocity(), m_Physics.GetAngularVelocity());
				m_fLastSendTime = curTime;
			}
		} else {
			if (curStateTime != prevStateTime)
			{
				float percentReplay = (curTime - curStateTime) / m_fUpdateDelay;
				percentReplay = Math.Clamp(percentReplay, 0, 1);
				vector ds = curState[3] - prevState[3];
				vector dv = curVelocity - prevVelocity;
				vector dw = curAngularVelocity - prevAngularVelocity;
				
				vector prevRot[3], curRot[3], outRot[3];
				prevRot[0] = prevState[0];
				prevRot[1] = prevState[1];
				prevRot[2] = prevState[2];
				
				curRot[0] = curState[0];
				curRot[1] = curState[1];
				curRot[2] = curState[2];
				
				float q1[4], q2[4], qout[4];
				Math3D.MatrixToQuat(prevRot, q1);
				Math3D.MatrixToQuat(curRot, q2);
				Math3D.QuatLerp(qout, q1, q2, percentReplay);
				Math3D.QuatToMatrix(qout, outRot);
				
				vector interpolatedMat[4]; 
				Math3D.MatrixIdentity4(interpolatedMat);
				interpolatedMat[0] = outRot[0];
				interpolatedMat[1] = outRot[1];
				interpolatedMat[2] = outRot[2];
				interpolatedMat[3] = prevState[3] + ds*percentReplay;
				
				m_Owner.SetTransform(interpolatedMat);
				m_Physics.SetVelocity(prevVelocity + dv*percentReplay);
				m_Physics.SetAngularVelocity(prevAngularVelocity + dw*percentReplay);
			}
		}
	}
	
	void UpdateStateInformation(vector mat[4], vector velocity, vector angularVelocity)
	{
		if (m_RplComponent.IsOwner())
			return;
		
		m_Physics.SetVelocity(velocity);
		m_Physics.SetAngularVelocity(angularVelocity);
		
		if (!m_bInterpolate)
		{
			m_Owner.SetTransform(mat);
		} else {
			if (System.GetTickCount() != curStateTime)
			{
				prevState = curState;
				prevStateTime = curStateTime;
				prevVelocity = curVelocity;
				prevAngularVelocity = curAngularVelocity;
			}
				
			curState = mat;
			curStateTime = System.GetTickCount();
			curVelocity = velocity;
			curAngularVelocity = angularVelocity;
		}
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void Rpc_Server_ReceiveNewStates(vector mat[4], vector velocity, vector angularVelocity)
	{
		UpdateStateInformation(mat, velocity, angularVelocity);
		Rpc(Rpc_Broadcast_ReceiveNewStates, mat, velocity, angularVelocity);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void Rpc_Broadcast_ReceiveNewStates(vector mat[4], vector velocity, vector angularVelocity)
	{
		UpdateStateInformation(mat, velocity, angularVelocity);
	}
}