// High -- gain potential energy while we have thrust to be able to glide towards target
// Direct -- go straight at target
// Low -- terrain following 500m target altitude (Avoid SAM)
enum EMissileTargetingMode {
	DirectPointing,
	DirectVelocity,
	Trajectory,
	High,
	Low
}

[BaseContainerProps()]
class R3D_MissileTriggerMode
{
	bool CanTrigger(R3D_MissileGuidanceComponent missile, vector target);
}

[BaseContainerProps()]
class R3D_MissileTriggerModeDistance: R3D_MissileTriggerMode
{
	[Attribute()]
	protected float m_Distance;
	
	override bool CanTrigger(R3D_MissileGuidanceComponent missile, vector target)
	{
		float distance = (missile.GetOwner().GetOrigin() - target).Length();
		if (distance <= m_Distance)
			return true;
		
		return false;
	}
}

[BaseContainerProps()]
class R3D_MissileTriggerModeCollision: R3D_MissileTriggerMode
{
	override bool CanTrigger(R3D_MissileGuidanceComponent missile, vector target)
	{
		vector missilePos = missile.GetOwner().GetOrigin();
		float missileHeight = missilePos[1];
		float terrainHeight = SCR_TerrainHelper.GetTerrainY(missilePos);
		
		return (missile.m_DidCollide) || missileHeight <= terrainHeight;
	}
}

class R3D_MissileGuidanceComponentClass: ScriptComponentClass {}
class R3D_MissileGuidanceComponent: ScriptComponent
{
	protected IEntity m_Owner;
	protected R3D_RocketMoveComponent m_Rocket;
	bool m_DidCollide;
	
	[Attribute(uiwidget: UIWidgets.Auto)]
	protected ref R3D_MissileTriggerMode m_TriggerMode;
	
	[Attribute(uiwidget: UIWidgets.Auto)]
	protected vector m_TargetPos;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMissileTargetingMode))]
	protected EMissileTargetingMode m_TargetingMode;
	
	vector GetTargetPosition()
	{
		return m_TargetPos;
	}
	
	void SetTargetPosition(vector pos)
	{
		m_TargetPos = pos;
	}
	
	protected float previousErrorX = 0;
	protected ref array<float> errorHistoryX = {};
	
	protected float previousErrorY = 0;
	protected ref array<float> errorHistoryY = {};
	
	[Attribute(category: "Direct Guidance")]
	protected float KpX;
	
	[Attribute(category: "Direct Guidance")]
	protected float KiX;
	
	[Attribute(category: "Direct Guidance")]
	protected float KdX;
	
	[Attribute(category: "Direct Guidance")]
	protected float KpY;
	
	[Attribute(category: "Direct Guidance")]
	protected float KiY;
	
	[Attribute(category: "Direct Guidance")]
	protected float KdY;
	
	void TriggerDetonation()
	{
		PrintFormat("boom %1m", minDistance);
		
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(m_Owner.FindComponent(BaseTriggerComponent));
		if (trigger) trigger.OnUserTrigger(m_Owner);
	}
	
	float minDistance = 1e10;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		vector missilePosition = m_Owner.GetOrigin();	
		vector posError = m_TargetPos - missilePosition;	
		
		Shape.CreateArrow(missilePosition, m_TargetPos, 0.1, Color.MAGENTA, ShapeFlags.ONCE);
			
		float errorX, errorY, uX, uY, errorIntegralX, errorIntegralY = 0;
		for (int i = 0; i < errorHistoryX.Count(); i++) errorIntegralX += errorHistoryX[i];
		for (int i = 0; i < errorHistoryY.Count(); i++) errorIntegralY += errorHistoryY[i];
		switch (m_TargetingMode) {
			case EMissileTargetingMode.DirectPointing:
			{
				vector desiredPointingDirection = posError.Normalized();
				vector pointingDirection = m_Owner.VectorToParent("0 0 1");
				vector errorDirection = desiredPointingDirection - pointingDirection;
				vector localErrorDirection = m_Owner.VectorToLocal(errorDirection);
				
				errorX = localErrorDirection[0];
				errorY = -localErrorDirection[1];
				
				break;
			}
			case EMissileTargetingMode.DirectVelocity:
			{
				// Just because we are pointing at the target, doesn't mean we are going to hit the target
				// Need to add a way to follow waypoints, or a precomputed trajectory. 
				// auto-updating trajectory would be cool too
				vector velocity = m_Owner.GetPhysics().GetVelocity();
				vector errorVelDir = (posError - velocity).Normalized();
				vector localError = m_Owner.VectorToLocal(errorVelDir);	
							
				errorX = localError[0];
				errorY = -localError[1];
				
				break;
			}
			case EMissileTargetingMode.Trajectory:
			{
				// error left-right should be based on velocity
				vector velocity = m_Owner.GetPhysics().GetVelocity();
				vector errorVelDir = (posError - velocity).Normalized();
				vector localError = m_Owner.VectorToLocal(errorVelDir);	
				errorX = localError[0];
				
				// pitch error should be adjusted based on error in estimated collision and desired collision point
				vector collision = m_Rocket.CalculateTrajectoryCollision(m_Owner);
				vector collisionError = m_TargetPos - collision;
				vector localErrorCollision = m_Owner.VectorToLocal(collisionError);
				float dotProd = vector.Dot(localErrorCollision, "0 0 1");
				Print(dotProd);	
				if (dotProd > 0) dotProd = 0;	
				
				errorY = dotProd;
				
				Shape.CreateSphere(COLOR_RED, ShapeFlags.ONCE, collision, 1);
				Shape.CreateArrow(collision, collision + collisionError, 0.1, COLOR_BLUE, ShapeFlags.ONCE);
			}
			case EMissileTargetingMode.High:
			{
				
				break;
			}
			case EMissileTargetingMode.Low:
			{
				
				break;
			}
		}
				
		float PX = KpX * errorX;
		float IX = KiX * errorIntegralX;
		float DX = KdX * (errorX - previousErrorX)/timeSlice;
		uX = PX + IX + DX;
		
		float PY = KpY * errorY;
		float IY = KiY * errorIntegralY;
		float DY = KdY * (errorY - previousErrorY)/timeSlice;
		uY = PY + IY + DY;
		
		previousErrorX = errorX;
		previousErrorY = errorY;
		
		if (m_Rocket.GetTimeUntilBurnout() > 0)
		{
			// Thrust Vectoring
			m_Rocket.SetThrustAngleX(uY);
			m_Rocket.SetThrustAngleY(uX);
		} else {
			// Map control signal to control surfaces to glide to destination
			
		}
				
		// Integral stuff
		//errorHistoryX.InsertAt(0, errorX * timeSlice);
		//errorHistoryY.InsertAt(0, errorY * timeSlice);
		
		if (errorHistoryX.Count() > 1000)
			errorHistoryX.Resize(1000);
		
		if (errorHistoryY.Count() > 1000)
			errorHistoryY.Resize(1000);
		
		float distance = posError.Length();
		if (Math.AbsFloat(distance) < minDistance) minDistance = distance;
		
		// DEBUG
#ifdef WORKBENCH
		vector pos = m_Owner.GetOrigin();
		DbgUI.Begin("Direct Guidance", 0, 0);
		
		DbgUI.Text(string.Format("errorX: %1", errorX));
		DbgUI.PlotLive("errorX", 500, 250, errorX, timeSlice, 1000);
		
		DbgUI.Text(string.Format("errorY: %1", errorY));
		DbgUI.PlotLive("errorY", 500, 250, errorY, timeSlice, 1000);
		
		DbgUI.Text(string.Format("minDistance: %1", Math.Round(minDistance*1000)/1000));
		DbgUI.PlotLive("distance", 500, 250, distance, timeSlice, 1000);
		
		DbgUI.End();
#endif
		
		float terrainHeight = SCR_TerrainHelper.GetTerrainY(missilePosition);
		float missileHeight = missilePosition[1];
		
		if (missileHeight <= terrainHeight)
		{
			missilePosition[1] = terrainHeight;
			m_Owner.SetOrigin(missilePosition);
		}
		
		if (m_TriggerMode.CanTrigger(this, m_TargetPos)) TriggerDetonation();
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		m_DidCollide = true;
	}
	
	override void EOnInit(IEntity owner)
	{
		m_Owner = owner;
		m_Rocket = R3D_RocketMoveComponent.Cast(owner.FindComponent(R3D_RocketMoveComponent));
	}
	
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.CONTACT);
	}
}