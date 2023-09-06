[BaseContainerProps()]
class ADM_Wing {
	[Attribute("0 0 0", UIWidgets.EditBox, params: "inf inf 0 purpose=coords space=entity")]
	vector m_vRootPosition;
	
	[Attribute()]
	ref array<ref ADM_WingSection> m_Sections;
	
	protected float m_fAspectRatio;
	void SetAspectRatio(float f) { m_fAspectRatio = f; }
	float GetAspectRatio() { return m_fAspectRatio; }
}