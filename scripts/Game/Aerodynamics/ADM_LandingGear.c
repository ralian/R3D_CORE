[BaseContainerProps()]
class ADM_LandingGear: Managed
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
}