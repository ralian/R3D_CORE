[BaseContainerProps()]
class ADM_WingSection 
{
	[Attribute("1", UIWidgets.EditBox, "Length of wing section in meters")]
	float m_Span;
	
	[Attribute("1", UIWidgets.EditBox, "Width of wing in meters")]
	float m_Chord;
	
	[Attribute(defvalue: "0.25", params: "0 1 0.05", uiwidget: UIWidgets.Slider, desc: "Position of aerodynamic center line in terms of x/c")]
	float m_ACPosition;
	
	[Attribute("0", UIWidgets.EditBox, "Angle of twist relative to body in degrees")]
	float m_TwistAngle;
	
	[Attribute("0", UIWidgets.EditBox, "Angle of sweep relative to body in degrees")]
	float m_SweepAngle;
	
	[Attribute("0", UIWidgets.EditBox, "Angle of wing relative to horizontal in degrees. (Make negative for anhedral)")]
	float m_DihedralAngle;
	
	[Attribute(uiwidget: UIWidgets.GraphDialog, params: "180 10 -90 -5")]
	ref Curve m_vLiftCurve;
	
	[Attribute(uiwidget: UIWidgets.GraphDialog, params: "180 4 -90 -2")]
	ref Curve m_vDragCurve;
	
	[Attribute(UIWidgets.Object)]
	ref array<ref ADM_ControlSurface> m_ControlSurfaces;
	
	vector m_vNormal; // relative to body frame
	vector m_vSpan; // relative to body frame
	vector m_vChord; // relative to body frame
	vector m_vAerodynamicCenter; // relative to body frame
	float m_fSurfaceArea;
	
	void SetNormalVector(vector n) { m_vNormal = n; }
	void SetSpanVector(vector c) { m_vSpan = c; }
	void SetChordVector(vector c) { m_vChord = c; }
	void SetAerodynamicCenter(vector ac) { m_vAerodynamicCenter = ac; }
	void SetSurfaceArea(float f) { m_fSurfaceArea = f; }
}