[ComponentEditorProps(category: "Game/", description: "Manager component allowing access and API over AdvanceAndSecure areas.")]
class GC_AdvanceAndSecureManagerClass : ScriptComponentClass
{
}

//! TILW_MissionEvent is a basic mission event. When its expression becomes true, it executes all instructions, then deactivates itself.
[BaseContainerProps(), BaseContainerCustomTitleField("m_name")]
class GC_AASOrder
{
	[Attribute("Faction", UIWidgets.Auto, desc: "The faction this order of capturing objectives applies to.")]
	string m_faction;
	
	[Attribute("", UIWidgets.Object, desc: "The order that objectives need to be captured in.")]
	ref array<string> m_order;
}

//! Capture & Hold manager that allows registration and management of areas.
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
}