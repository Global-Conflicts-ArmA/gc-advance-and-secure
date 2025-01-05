[ComponentEditorProps(category: "Game/", description: "Manager component allowing setting of capture order for Advance and Secure.")]
class GC_AdvanceAndSecureManagerClass : ScriptComponentClass
{
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_name")]
class GC_AASOrder
{
	[Attribute("Faction", UIWidgets.Auto, desc: "The faction this order of capturing objectives applies to.")]
	string m_faction;
	
	[Attribute("", UIWidgets.Object, desc: "The order that objectives need to be captured in.")]
	ref array<string> m_order;
	
	[Attribute("", UIWidgets.Object, desc: "If the game should end when all objectives are controlled by this faction.")]
	bool m_endGameOnAll;
}

//! Capture & Hold manager that handles objective ordering.
//! This component must be attached to a SCR_BaseGameMode entity!
//! There should only be a single manager at any given time.
class GC_AdvanceAndSecureManager : ScriptComponent
{
	//! Objective ordering per faction.
	[Attribute("", desc: "Objective ordering per faction.")]
	ref array<ref GC_AASOrder> m_factionOrders;
	//------------------------------------------------------------------------------------------------
	//! Initialize the manager.
	protected override void EOnInit(IEntity owner)
	{
		return super.EOnInit(owner);
	}
	
	bool AASOrdersComplete(Faction faction)
	{
		string fkey = faction.GetFactionKey();
		
		for (int i = 0; i < m_factionOrders.Count(); ++i)
		{
			if (m_factionOrders[i].m_faction != fkey)
			{
				continue;
			}

			if (!m_factionOrders[i].m_endGameOnAll)
			{
				return false;
			}

			bool aasComplete = true;

			// iterate through the entries in the faction objective order
			// check if the areas are owned in order, ending the game
			// when a faction has completed their full objective set
			for (int n = 0; n < m_factionOrders[i].m_order.Count(); ++n)
			{
				// loop has found an objective not controlled by this faction, so aas isn't complete
				GC_AdvanceAndSecureArea previousArea = GC_AdvanceAndSecureArea.Cast(GetGame().GetWorld().FindEntityByName(m_factionOrders[i].m_order[n]));
				if (!previousArea.GetOwningFaction())
				{
					aasComplete = false;
					break;
				}
				if (previousArea.GetOwningFaction().GetFactionKey() != fkey) 
				{
					aasComplete = false;
					break;
				}
			}
			return aasComplete;
		}
		return false;
	}
}