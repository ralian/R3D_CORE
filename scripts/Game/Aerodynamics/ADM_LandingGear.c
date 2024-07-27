[BaseContainerProps()]
class ADM_LandingGear
{
	[Attribute(params: "0 inf")]
	float m_fDragArea;
	
	[Attribute(params: "0 inf")]
	float m_fDragCoefficient;
	
	[Attribute()]
	string m_sSignal;
	
	int m_iSignalIndex = -1;
	
	[Attribute()]
	ref PointInfo m_vPosition;
	
	[Attribute(defvalue: "90")]
	float m_fRotationAngle;
	
	[Attribute(defvalue: "10")]
	float m_fRotationRate;
	
	protected bool m_bGearDeployed = true;
	protected float m_fCurrentState = 1;
	
	void SetGearDeployed(bool deploy)
	{
		m_bGearDeployed = deploy;
	}
	
	bool GetGearDeployed()
	{
		return m_bGearDeployed;
	}
	
	// 0 = retracted; 1 = deployed
	void SetState(float state)
	{
		m_fCurrentState = Math.Clamp(state, 0, 1);
	}
	
	float GetState()
	{
		return m_fCurrentState;
	}
	
	// amount is in degrees
	void RotateGear(float amount)
	{
		m_fCurrentState += amount / m_fRotationAngle;
		m_fCurrentState = Math.Clamp(m_fCurrentState, 0, 1);
	}
	
	void Update(float timeSlice)
	{
		if (m_fRotationRate <= 0 || m_fRotationAngle == 0)
		{
			return;
		}
			
		if (m_bGearDeployed && GetState() < 1)
		{
			RotateGear(m_fRotationRate * timeSlice);
		}
		
		if (!m_bGearDeployed && GetState() > 0)
		{
			RotateGear(-m_fRotationRate * timeSlice);
		}
	}
}