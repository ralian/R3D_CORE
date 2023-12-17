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
	
	[Attribute(desc: "Disable headlight when landing gear is more than 90% stowed.")]
	bool m_bDisableHeadlight;
	
	protected float m_fCurrentState = 1;
	
	// amount is in degrees
	void RotateGear(float amount)
	{
		m_fCurrentState += amount / m_fRotationAngle;
		m_fCurrentState = Math.Clamp(m_fCurrentState, 0, 1);
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
}