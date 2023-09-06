class R3D_UIInfoClass: ScriptComponentClass {}

class R3D_UIInfo: ScriptComponent
{
	[Attribute()]
	protected ref SCR_UIInfo m_uiInfo;
	
	SCR_UIInfo GetUIInfo()
	{
		return m_uiInfo;
	}
}