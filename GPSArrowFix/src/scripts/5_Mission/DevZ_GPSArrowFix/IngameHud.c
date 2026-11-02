// DayZ Expansion Navigation only (compiled out on servers and clients without it).
#ifdef EXPANSIONMODNAVIGATION

// Expansion's GPS HUD mini-map draws the player's arrow (m_PlayerArrowMarker). Every frame,
// IngameHud::Update() hides that arrow while a menu is open (inventory, map, trader, book...),
// so it doesn't draw over the menu. But the branch for "no menu open" also calls Hide() where
// it should show the arrow again:
//
//     if (GetGPSMapState() && !m_PlayerArrowMarker.GetLayoutRoot().IsVisible())
//         m_PlayerArrowMarker.Hide();
//
// So after the first menu the arrow stays hidden until something calls RefreshHudVisibility()
// (cycling the GPS with its key). The fix: after Expansion's Update(), show the arrow again
// whenever RefreshHudVisibility() itself would show it (same conditions) and it's hidden.
modded class IngameHud
{
	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (!m_PlayerArrowMarker || m_PlayerArrowMarker.GetLayoutRoot().IsVisible())
			return;

		// Expansion_CanShowHUDElements() is false while any menu or the inventory is open
		bool showArrow = Expansion_CanShowHUDElements() && m_ExpansionGPSState && m_ExpansionGPSMapState && m_ExpansionGPSSetting;
		if (showArrow && (m_ExpansionGPSPosSetting == 1 || m_ExpansionGPSPosSetting == 2))
			m_PlayerArrowMarker.Show(true);
	}
}

#endif
