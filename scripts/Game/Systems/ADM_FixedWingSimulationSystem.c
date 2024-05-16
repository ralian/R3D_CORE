class ADM_FixedWingSimulationSystem: GameSystem
{
	protected bool m_bUpdating = false;
	protected ref array<ADM_FixedWingSimulation> m_Components = {};
	protected ref array<ADM_FixedWingSimulation> m_DeletedComponents = {};
	
	override protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetPhysicsTimeSlice();
		
		m_bUpdating = true;
		foreach (ADM_FixedWingSimulation comp: m_Components)
		{
			comp.Simulate(timeSlice);
		}
		m_bUpdating = false;
		
		foreach (ADM_FixedWingSimulation comp: m_DeletedComponents)
		{
			m_Components.RemoveItem(comp);
		}
		m_DeletedComponents.Clear();
	}
	
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("ADM_FixedWingSimulationSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (ADM_FixedWingSimulation comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(ADM_FixedWingSimulation component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(ADM_FixedWingSimulation component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		if (m_bUpdating)
			m_DeletedComponents.Insert(component);
		else
			m_Components.Remove(idx);
	}
}