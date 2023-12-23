[BaseContainerProps()]
class ADM_Wing {
	[Attribute("0 0 0", UIWidgets.EditBox, params: "inf inf 0 purpose=coords space=entity")]
	vector m_vRootPosition;
	
	[Attribute()]
	ref array<ref ADM_WingSection> m_Sections;
	
	protected float m_fSpan;
	protected float m_fChord;
	protected float m_fAspectRatio;
	protected float m_fSurfaceArea;
	protected vector m_vAerodynamicCenter;

	float GetAspectRatio() 
	{ 
		return m_fAspectRatio; 
	}
	
	float GetSurfaceArea()
	{
		return m_fSurfaceArea;
	}
	
	vector GetAerodynamicCenter()
	{
		return m_vAerodynamicCenter;
	}
	
	void CalculatePanels()
	{			
		// Each subsequent section should start at the leading edge at the end of the span of the previous section
		vector previousSectionLE = vector.Zero;
		float totalSpan = 0;
		float meanChord = 0;
		vector acTotal = vector.Zero;
		float acTotalArea = 0;
		for (int j = 0; j < m_Sections.Count(); j++)
		{
			ADM_WingSection curSection = m_Sections[j];
			
			// Transformations on basis vectors
			// Rotate points for twist and dihedral angles
			vector angles;
			angles[1] = curSection.m_TwistAngle;
			angles[2] = curSection.m_DihedralAngle;
			
			// Skew points for sweep angle
			vector transformMatrix[3];
			Math3D.AnglesToMatrix(angles, transformMatrix);
			transformMatrix[0][2] = Math.Tan(curSection.m_SweepAngle * Math.DEG2RAD);
			
			vector vSpan = vector.Right.Multiply3(transformMatrix); // 1 0 0
			vector vChord = vector.Forward.Multiply3(transformMatrix); // 0 0 1
			vector vNormal = vChord * vSpan;
			
			// AC Line
			vector acLine[2];
			acLine[0] = m_vRootPosition + previousSectionLE - vChord * curSection.m_ACPosition * curSection.m_Chord;
			acLine[1] = acLine[0] + vSpan * curSection.m_Span;
			previousSectionLE = previousSectionLE + vSpan * curSection.m_Span;
			vector acCenter = (acLine[0] + acLine[1])/2;
			
			totalSpan += Math.AbsFloat(curSection.m_Span);
			meanChord += Math.AbsFloat(curSection.m_Chord);
			
			float surfaceArea = Math.AbsFloat(curSection.m_Span * curSection.m_Chord);
			acTotal += acCenter * surfaceArea;
			acTotalArea += surfaceArea;
	
			curSection.SetNormalVector(vNormal);
			curSection.SetSpanVector(vSpan);
			curSection.SetChordVector(vChord);
			curSection.SetAerodynamicCenter(acCenter);
			curSection.SetSurfaceArea(surfaceArea);
		}
		
		if (m_Sections && m_Sections.Count() > 0)
		{
			meanChord /= m_Sections.Count();
		}
			
		if (meanChord > 0)
			m_fAspectRatio = totalSpan / meanChord;
		
		if (acTotalArea > 0) 
			acTotal /= acTotalArea;
		
		m_vAerodynamicCenter = acTotal;
		m_fSurfaceArea = acTotalArea;
	}
}