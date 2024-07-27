class NwkPlaneMovementComponentClass: ScriptGameComponentClass {}

class NwkPlaneMovementComponent : ScriptGameComponent
{
	protected IEntity m_Owner;
	protected Physics m_Physics;
	protected RplComponent m_RplComponent;
	protected NwkCarMovementComponent m_NwkCar;
	
	override void OnPostInit(IEntity owner)
	{
		m_Owner = owner;
		m_Physics = owner.GetPhysics();
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_NwkCar = NwkCarMovementComponent.Cast(owner.FindComponent(NwkCarMovementComponent));
		
		if (m_NwkCar)
		{
			m_NwkCar.SetPrediction(false);
			m_NwkCar.SetAllowance(false, 100, 100, 100, 100);
			m_NwkCar.EnableInterpolation(true);
		}
		
		SetEventMask(owner, EntityEvent.POSTFRAME);	
	}
	
	private float m_fLastSendTime = -float.MAX;
	private float m_fUpdateDelay = 20;
	override event protected void EOnPostFrame(IEntity owner, float timeSlice)
	{
		super.EOnPostFrame(owner, timeSlice);
		if (!m_RplComponent.IsOwner())
			return;
		
		float curTime = System.GetTickCount();
		float dtSend = curTime - m_fLastSendTime;
		if (dtSend > m_fUpdateDelay)
		{
			vector mat[4];
			owner.GetTransform(mat);
			Rpc(Rpc_Server_ReceiveNewStates, mat, m_Physics.GetVelocity(), m_Physics.GetAngularVelocity());
			m_fLastSendTime = curTime;
		}
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Server)]
	void Rpc_Server_ReceiveNewStates(vector mat[4], vector velocity, vector angularVelocity)
	{
		m_Owner.SetTransform(mat);
		m_Physics.SetVelocity(velocity);	
		m_Physics.SetAngularVelocity(angularVelocity);
	}
}