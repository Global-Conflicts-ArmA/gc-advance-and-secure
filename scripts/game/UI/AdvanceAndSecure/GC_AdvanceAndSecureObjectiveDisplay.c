//! This InfoDisplay allows drawing of individual HUD objective elements for individual
//! Advance & Secure areas by projecting their position from world to screen.
class GC_AdvanceAndSecureObjectiveDisplay : SCR_CaptureAndHoldObjectiveDisplay
{
	//------------------------------------------------------------------------------------------------
	//! Creates and fills the objective collection using provided areas.
	protected override void CreateObjectiveDisplays(array<SCR_CaptureAndHoldArea> areas)
	{
		Widget objective;
		SCR_CaptureAndHoldObjectiveDisplayObject displayObject;
		for (int i = 0, count = areas.Count(); i < count; i++)
		{
			objective = GetRootWidget().GetWorkspace().CreateWidgets(m_rObjectiveHUDLayout, GetRootWidget());
			if (!objective)
				continue;

			displayObject = new GC_AdvanceAndSecureObjectiveDisplayObject(objective, areas[i]);
			m_aObjectiveElements.Insert(displayObject);
		}
	}
}