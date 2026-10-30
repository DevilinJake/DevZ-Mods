// Barrels keep every vanilla rule (empty and closed to carry or put in cargo, no barrels in
// barrels). The rack only adds two actions and fixes the open/closed state on clients.
modded class Barrel_ColorBase
{
	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionDetach);		// take an empty barrel off the rack into your hands
		AddAction(ActionAttachOnProxy);	// put the barrel in your hands onto the rack
	}

	// Vanilla opens every barrel the moment it is attached (meant for truck barrels). Clients
	// re-run this when the rack streams back into view, so they saw rack barrels as open while
	// the server had them closed. On clients, keep the state the server sent instead.
	override void OnWasAttached(EntityAI parent, int slot_id)
	{
		bool wasOpen = IsOpen();

		super.OnWasAttached(parent, slot_id);

		if (g_Game.IsDedicatedServer() || !parent || !parent.IsInherited(Invictuz_Barrel_Storage))
			return;

		if (!wasOpen)
		{
			m_Openable.Close();
			SetTakeable(true);
			UpdateVisualState();
		}
	}
};
