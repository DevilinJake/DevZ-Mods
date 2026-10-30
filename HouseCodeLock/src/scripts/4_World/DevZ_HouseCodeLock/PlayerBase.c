modded class PlayerBase
{
	// The lock actions are the player's, like the vanilla door actions, so they work on any part
	// of the door and come after Open / Close in the list (a player who knows the code sees
	// "Open locked door" first, like on an Expansion gate)
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(DevZ_ActionHouseCodeLock, InputActionMap);
		AddAction(DevZ_ActionHouseChangeCode, InputActionMap);
	}
}
