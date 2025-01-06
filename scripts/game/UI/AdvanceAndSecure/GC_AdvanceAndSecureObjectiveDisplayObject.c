//! This object serves as a wrapper for individual HUD objective widgets.
class GC_AdvanceAndSecureObjectiveDisplayObject : SCR_CaptureAndHoldObjectiveDisplayObject
{	
	protected GC_AdvanceAndSecureArea m_AffiliatedAASArea;
	
	//------------------------------------------------------------------------------------------------
	//! Create new wrapper for objective display.
	void GC_AdvanceAndSecureObjectiveDisplayObject(notnull Widget root, notnull SCR_CaptureAndHoldArea area)
	{
		m_AffiliatedAASArea = GC_AdvanceAndSecureArea.Cast(area);
	}

	//------------------------------------------------------------------------------------------------
	//! Update this widget as dynamic UI element projected to screen space
	override void UpdateDynamic(IEntity playerEntity, float timeSlice)
	{
		super.UpdateDynamic(playerEntity, timeSlice);
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(playerEntity);
		if (!m_AffiliatedAASArea.CappableByFaction(char.GetFaction()))
		{
			m_wRoot.SetVisible(false);
		}
	}
}