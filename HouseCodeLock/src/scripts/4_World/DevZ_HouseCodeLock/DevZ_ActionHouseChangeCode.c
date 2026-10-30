// Looking at a house door whose Expansion code lock is unlocked (or at the lock itself):
// "Change code" (Expansion's keypad). All zeroes clears the code, which frees the lock to be
// taken off, the same as on a gate. A player action (PlayerBase.c).
class DevZ_ActionHouseChangeCode: ActionInteractBase
{
	// Only used on the player's side (IsLocal)
	protected ExpansionCodeLock m_DevZ_CodeLock;

	void DevZ_ActionHouseChangeCode()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_HUDCursorIcon = CursorIcons.CloseDoors;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
	}

	override string GetText()
	{
		return "#STR_EXPANSION_BB_CODE_LOCK_CHANGE";
	}

	override bool IsLocal()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_DevZ_CodeLock = null;

		if (!GetExpansionSettings().GetBaseBuilding(false).IsLoaded())
			return false;

		BuildingBase house;
		int doorIndex;
		if (!DevZ_HouseCodeLock.ResolveDoor(target, house, doorIndex, m_DevZ_CodeLock) || !m_DevZ_CodeLock)
			return false;

		if (!DevZ_HouseCodeLock.LockActionsAllowed(target, m_DevZ_CodeLock))
			return false;

		// Only with the door shut, like Expansion on a gate: the lock goes red every time the
		// door is opened, so with it open anyone could walk up and change the code
		if (house.IsDoorOpen(doorIndex))
			return false;

		return m_DevZ_CodeLock.HasCode() && !m_DevZ_CodeLock.ExpansionIsLocked();
	}

	override void Start(ActionData action_data)
	{
		super.Start(action_data);

		if (!IsMissionClient() || !m_DevZ_CodeLock)
			return;

		ExpansionCodeLockUI menu = ExpansionCodeLockUI.Cast(g_Game.GetUIManager().EnterScriptedMenu(MENU_EXPANSION_CODELOCK_MENU, null));
		if (menu)
		{
			menu.SetChangeCodelock(true);
			menu.SetConfirm(true);
			menu.SetTarget(m_DevZ_CodeLock, "");
		}
	}
}
