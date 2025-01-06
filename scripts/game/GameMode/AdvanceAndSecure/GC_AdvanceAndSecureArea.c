[EntityEditorProps(category: "Game/", description: "Area that awards score points when captured.")]
class GC_AdvanceAndSecureAreaClass : SCR_CaptureAndHoldAreaClass
{
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_name")]
class GC_AASDefenderGroup
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab to spawn", "et")]
	ResourceName m_prefab;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Waypoint prefab to use", "et")]
	ResourceName m_waypoint;
	
	[Attribute("", UIWidgets.Range, "Radius of defence", "et")]
	float m_radius;
	
	void SpawnGroup(IEntity owner)
	{
		if (!Replication.IsServer()) return;
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		vector mat[4];
		owner.GetWorldTransform(mat);
		spawnParams.Transform = mat;
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(Resource.Load(m_prefab), GetGame().GetWorld(), spawnParams);
		
		if (m_waypoint)
		{
			IEntity spawnedWaypoint = GetGame().SpawnEntityPrefab(Resource.Load(m_waypoint), GetGame().GetWorld(), spawnParams);
		
			AIWaypoint wp = AIWaypoint.Cast(spawnedWaypoint);
			wp.SetCompletionRadius(m_radius);
		
			AIGroup grp = AIGroup.Cast(spawnedEntity);
			grp.AddWaypoint(wp);
		}
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_name")]
class GC_AASCounterattack
{
	[Attribute("", UIWidgets.Auto, desc: "When the area is captured by this faction, spawn the counterattack")]
	string m_factionKey;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab to spawn", "et")]
	ResourceName m_prefab;
	
	[Attribute("", UIWidgets.Auto, desc: "Name of existing entity at which the prefab should be spawned")]
	string m_locationName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Waypoint prefab to use", "et")]
	ResourceName m_waypoint;
	
	[Attribute("", UIWidgets.Range, "Radius of defence", "et")]
	float m_radius;
	
	protected bool triggered;
	
	void SpawnGroup(IEntity owner)
	{
		if (!Replication.IsServer()) return;
		if (triggered) return;
		IEntity e = GetGame().GetWorld().FindEntityByName(m_locationName);
		if (!e) return;
		
		EntitySpawnParams groupSpawnParams = new EntitySpawnParams();
		vector groupMat[4];
		e.GetWorldTransform(groupMat);
		groupSpawnParams.Transform = groupMat;
		
		EntitySpawnParams waypointSpawnParams = new EntitySpawnParams();
		vector waypointMat[4];
		owner.GetWorldTransform(waypointMat);
		waypointSpawnParams.Transform = waypointMat;
		
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(Resource.Load(m_prefab), GetGame().GetWorld(), groupSpawnParams);
		IEntity spawnedWaypoint = GetGame().SpawnEntityPrefab(Resource.Load(m_waypoint), GetGame().GetWorld(), waypointSpawnParams);
		
		AIWaypoint wp = AIWaypoint.Cast(spawnedWaypoint);
		wp.SetCompletionRadius(m_radius);
		
		AIGroup grp = AIGroup.Cast(spawnedEntity);
		grp.AddWaypoint(wp);
		
		triggered = true;
	}
}

//! This area awards score to the faction which controls the area periodically.
//! Area registers and unregisters from the SCR_AdvanceAndSecureManager.
class GC_AdvanceAndSecureArea : SCR_CaptureAndHoldArea
{
	//! Spawn points related to this AAS area.
	[Attribute("", desc: "Spawn points related to control of this area.")]
	ref array<string> m_spawnPoints;
	
	//! AI Defenders related to this AAS area.
	[Attribute("", desc: "Groups spawned to defend this area at game start.")]
	ref array<ref GC_AASDefenderGroup> m_defenderGroups;
	
	//! AI Counterattacks related to this AAS area.
	[Attribute("", desc: "Groups spawned to counterattack this area when it is captured.")]
	ref array<ref GC_AASCounterattack> m_counterattacks;
	
	//------------------------------------------------------------------------------------------------
	//! Initialize this area and register it to parent manager.
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		if (!GetGame().InPlayMode())
			return;
		
		if (m_defenderGroups) {
			for (int i = 0; i < m_defenderGroups.Count(); ++i)
			{
				m_defenderGroups[i].SpawnGroup(this);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns control of the previous objective in the AAS order for [faction].
		\param Faction The faction to return the count for
		\return Returns bool true or false ownership of previous objective
	*/
	protected bool CappableByFaction(Faction faction)
	{
		string fkey = faction.GetFactionKey();
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return true;

		GC_AdvanceAndSecureManager parentManager = GC_AdvanceAndSecureManager.Cast(gameMode.FindComponent(GC_AdvanceAndSecureManager));
		if (!parentManager)
		{
			return true;
		}
		
		array<ref GC_AASOrder> factionOrders = parentManager.m_factionOrders;
		
		for (int i = 0; i < factionOrders.Count(); ++i)
		{
			if (factionOrders[i].m_faction == fkey)
			{
				// iterate through the entries in the faction objective order
				// check if the areas are owned in order, returning when either
				// false or the current area is found
				for (int n = 0; n < factionOrders[i].m_order.Count(); ++n)
				{
					// loop reached this area, so it's cappable
					if (factionOrders[i].m_order[n] == GetName())
					{
						return true;
					}

					// loop has found an objective not controlled by this faction, so it's not cappable
					GC_AdvanceAndSecureArea previousArea = GC_AdvanceAndSecureArea.Cast(GetGame().GetWorld().FindEntityByName(factionOrders[i].m_order[n]));
					if (!previousArea.m_pOwnerFaction)
					{
						return false;
					}
					if (previousArea.m_pOwnerFaction.GetFactionKey() != fkey) 
					{
						return false;
					}
				}
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles owning and contesting changes based on defined parameters.
	protected override Faction EvaluateOwnerFaction()
	{
		Faction owningFaction = super.EvaluateOwnerFaction();

		// do not update ownership if the new evaluated faction (owningFaction)
		// does not control the previous objective in its objective order
		if (owningFaction)
		{
			if (!CappableByFaction(owningFaction))
			{
				owningFaction = m_pOwnerFaction;
			}
		}
		
		if (IsContested())
		{
			// disable all spawn points controlled by objective
			for (int i = 0; i < m_spawnPoints.Count(); ++i)
			{
				SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(GetGame().GetWorld().FindEntityByName(m_spawnPoints[i]));
				spawnPoint.SetSpawnPointEnabled_S(false);
			}
		}
		else
		{
			// enable/disable spawn points controlled by objective depending on owner
			for (int i = 0; i < m_spawnPoints.Count(); ++i)
			{
				SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(GetGame().GetWorld().FindEntityByName(m_spawnPoints[i]));
				spawnPoint.SetSpawnPointEnabled_S(owningFaction && spawnPoint.GetFactionKey() == owningFaction.GetFactionKey());
			}
		}

		return owningFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Occurs on change of owning faction of the area.
	//! \param previousFaction Faction which held the point prior to this change or null if none.
	//! \param newFaction Faction that holds the point after this change or null if none.
	protected override void OnOwningFactionChanged(Faction previousFaction, Faction newFaction)
	{
		super.OnOwningFactionChanged(previousFaction, newFaction);
		
		if (m_counterattacks)
		{
			for (int i = 0; i < m_counterattacks.Count(); ++i)
			{
				if (newFaction.GetFactionKey() == m_counterattacks[i].m_factionKey)
				{
					m_counterattacks[i].SpawnGroup(this);
				}
			}
		}

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		if (!gameMode.IsRunning())
			return;

		GC_AdvanceAndSecureManager parentManager = GC_AdvanceAndSecureManager.Cast(gameMode.FindComponent(GC_AdvanceAndSecureManager));
		if (!parentManager)
		{
			return;
		}

		if (parentManager.AASOrdersComplete(newFaction))
		{
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(newFaction);
			SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: factionIndex);
			gameMode.EndGameMode(endData);
		}
	}
}