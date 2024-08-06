[BaseContainerProps()]
class ADM_ControlSurface
{
	[Attribute("", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ADM_InputType))]
	ADM_InputType m_Type;
	
	[Attribute()]
	bool m_bInvertInput;
	
	[Attribute(desc: "Automatically return to zero when no input", defvalue: "1")]
	bool m_bAutoZero;
	
	[Attribute(params: "0 1", desc: "Width of control surface in terms of % chord")]
	float m_fChordPercent;
	
	// degrees
	[Attribute(desc: "degrees")]
	float m_fMinAngle;
	
	// degrees
	[Attribute(desc: "degrees")]
	float m_fMaxAngle;
	
	[Attribute(desc: "degrees/sec")]
	float m_fMaxActuationRate;
	
	[Attribute()]
	string m_sSignal;
	
	int m_iSignalIndex = -1;
	float m_fCurAngle = 0;
	float m_fZeroAngle = 0; // zero input angle, trim
	
	// input from -1 to 1
	// output is in degrees
	/*float GetAngle(float input)
	{
		if (m_bInvertInput)
			input *= -1;
		
		if (input < 0)
			return (input*-1) * m_fMinAngle;
		else
			return input * m_fMaxAngle;
	}*/
	
	void Update(float timeSlice, float input)
	{
		if (m_bInvertInput)
			input *= -1;
		
		float targetAngle = m_fMaxAngle;
		if (input < 0)
			targetAngle = m_fMinAngle;
		
		if (input == 0 && m_bAutoZero)
			targetAngle = m_fZeroAngle;
		
		if (input == 0 && !m_bAutoZero)
			targetAngle = m_fCurAngle;
		
		targetAngle *= Math.AbsFloat(input);
		
		float delta = m_fMaxActuationRate*timeSlice;
		float error = targetAngle - m_fCurAngle;
		if (delta > Math.AbsFloat(error))
			delta = Math.AbsFloat(error);
		
		if (error < 0)
			delta *= -1;
		
		m_fCurAngle += delta;
		m_fCurAngle = Math.Clamp(m_fCurAngle, m_fMinAngle, m_fMaxAngle);
	}
	
	float GetAngle()
	{
		return m_fCurAngle;
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