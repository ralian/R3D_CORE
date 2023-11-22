[BaseContainerProps()]
class ADM_ControlSurface
{
	[Attribute("", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ADM_InputType))]
	ADM_InputType m_Type;
	
	[Attribute()]
	bool m_bInvertInput;
	
	[Attribute(params: "0 1", desc: "Width of control surface in terms of % chord")]
	float m_fChordPercent;
	
	// degrees
	[Attribute()]
	float m_fMinAngle;
	
	// degrees
	[Attribute()]
	float m_fMaxAngle;
	
	[Attribute()]
	string m_sSignal;
	
	int m_iSignalIndex = -1;
	
	// input from -1 to 1
	// output is in degrees
	float GetAngle(float input)
	{
		if (m_bInvertInput)
			input *= -1;
		
		if (input < 0)
			return (input*-1) * m_fMinAngle;
		else
			return input * m_fMaxAngle;
	}
	
	// Control surfaces essentially change the camberline of the airfoil, resulting in a new AoA.
	float GetDeltaAoA(ADM_WingSection curSection, float deflectionAngle, float angleOfAttack) 
	{
		float dy = m_fChordPercent * Math.Tan(deflectionAngle * Math.DEG2RAD);
		float dAoA = Math.Atan2(dy, 1) * Math.RAD2DEG;
		
		return dAoA;
	}
	
	float GetDeltaCL(ADM_WingSection curSection, float deflectionAngle, float angleOfAttack)
	{
		return 0;
	}
	
	float GetDeltaCD(ADM_WingSection curSection, float deflectionAngle, float angleOfAttack)
	{
		return 0;
	}
}

[BaseContainerProps()]
class ADM_ControlSurfaceDragDevice: ADM_ControlSurface
{
	[Attribute(uiwidget: UIWidgets.GraphDialog, params: "90 4 0 -2")]
	ref Curve m_vDragCurve;
	
	override float GetDeltaCD(ADM_WingSection curSection, float deflectionAngle, float angleOfAttack)
	{
		float range = m_fMaxAngle - m_fMinAngle;
		float posInRange = m_fMinAngle + deflectionAngle;
		
		return Math3D.Curve(ECurveType.CurveProperty2D, posInRange / range, m_vDragCurve)[1];
	}
}


[BaseContainerProps()]
class ADM_ControlSurfaceFlap: ADM_ControlSurfaceDragDevice
{
	[Attribute(uiwidget: UIWidgets.GraphDialog, params: "90 4 0 -2")]
	ref Curve m_vLiftCurve;
	
	override float GetDeltaCL(ADM_WingSection curSection, float deflectionAngle, float angleOfAttack)
	{
		float range = m_fMaxAngle - m_fMinAngle;
		float posInRange = m_fMinAngle + deflectionAngle;
		
		return Math3D.Curve(ECurveType.CurveProperty2D, posInRange / range, m_vLiftCurve)[1];
	}
}