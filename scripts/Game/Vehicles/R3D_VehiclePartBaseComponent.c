class R3D_VehiclePartBaseComponentClass : ScriptComponentClass
{
	static override array<typename> Requires(IEntityComponentSource src)
	{
		array<typename> requires = new array<typename>;
		
		requires.Insert(R3D_VehicleSimulationComponent);
		
		return requires;
	}
};

class R3D_VehiclePartBaseComponent : ScriptComponent
{
	protected R3D_VehicleSimulationComponent m_Simulation;
	protected SignalsManagerComponent m_SignalsManager;
	protected BaseProcAnimComponent m_ProcAnim;
	
	vector GetBonePosition(string name) {
		vector mat[4];
		TNodeId idx = GetOwner().GetAnimation().GetBoneIndex(name);
		GetOwner().GetAnimation().GetBoneMatrix(idx,mat);
		return mat[0];
	}
	
	void GetBoneMatrix(string name,out vector mat[]) {
		TNodeId idx = GetOwner().GetAnimation().GetBoneIndex(name);
		GetOwner().GetAnimation().GetBoneMatrix(idx,mat);
	}

	bool OnInitialize(R3D_VehicleSimulationComponent simulation)
	{
		m_Simulation = simulation;

		m_SignalsManager = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		m_ProcAnim = BaseProcAnimComponent.Cast(GetOwner().FindComponent(BaseProcAnimComponent));

		return true;
	}

	void OnCompute(float dt, Physics physics)
	{

	}

	void OnSimulate(float dt, Physics physics)
	{

	}
};
