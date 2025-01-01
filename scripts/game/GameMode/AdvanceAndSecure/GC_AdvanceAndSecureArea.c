[EntityEditorProps(category: "Game/", description: "Area that awards score points when captured.")]
class GC_AdvanceAndSecureAreaClass : SCR_CaptureAndHoldAreaClass
{
}

//! This area awards score to the faction which controls the area periodically.
//! Area registers and unregisters from the SCR_AdvanceAndSecureManager.
class GC_AdvanceAndSecureArea : SCR_CaptureAndHoldArea
{
	//! Spawn points related to this AAS area.
	[Attribute("", desc: "Spawn points related to control of this area.")]
	ref array<string> m_spawnPoints;
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
			Print("AdvanceAndSecureArea cannot find GC_AdvanceAndSecureManager! Can't enforce AAS order!", LogLevel.WARNING);
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
				for (int n = 0; i < factionOrders[i].m_order.Count(); ++n)
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
}