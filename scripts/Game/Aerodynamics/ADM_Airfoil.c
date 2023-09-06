//! A brief explanation of what this component does.
//! The explanation can be spread across multiple lines.
//! This should help with quickly understanding the script's purpose.
[BaseContainerProps()]	
class ADM_Airfoil
{
	[Attribute(desc: "List of points that describe this airfoil cross section.")]
	ref array<ref vector> m_Geometry;
	
	static ADM_Airfoil GetConfig(string resourcePath)
	{
		Resource holder = BaseContainerTools.LoadContainer(resourcePath);
		if (holder)
		{
			ADM_Airfoil obj = ADM_Airfoil.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			if (obj)
			{
				return obj;
			}
		}
		
		return null;
	}
};