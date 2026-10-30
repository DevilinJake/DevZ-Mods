// Looking at an unlocked Expansion code lock on a house door: "Change code" (Expansion's keypad)
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
		m_ConditionTarget = new CCTCursorInherited({ExpansionCodeLock}, UAMaxDistances.DEFAULT);
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
		if (!GetExpansionSettings().GetBaseBuilding(false).IsLoaded())
			return false;

		m_DevZ_CodeLock = ExpansionCodeLock.Cast(target.GetObject());
		if (!m_DevZ_CodeLock || !m_DevZ_CodeLock.DevZ_GetHouse())
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
