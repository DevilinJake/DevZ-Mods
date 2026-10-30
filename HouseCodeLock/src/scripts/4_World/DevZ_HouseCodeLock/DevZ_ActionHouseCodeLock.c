// Looking at an Expansion code lock on a house door: Set code / Close & lock / Enter code /
// Unlock, the same choices and keypad Expansion gives on a gate. The keypad talks to the lock
// itself, so Expansion's own server checks (wrong-code damage, known players, logs) all apply.
class DevZ_ActionHouseCodeLock: ActionInteractBase
{
	// Only used on the player's side (IsLocal)
	protected ExpansionCodeLock m_DevZ_CodeLock;
	protected bool m_DevZ_IsKnownUser;

	void DevZ_ActionHouseCodeLock()
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
		if (!m_DevZ_CodeLock || !m_DevZ_CodeLock.HasCode())
			return "#STR_EXPANSION_BB_CODE_LOCK_SET_CODE";

		if (!m_DevZ_CodeLock.ExpansionIsLocked())
			return "#STR_EXPANSION_BB_CODE_CLOSE_LOCK";

		if (m_DevZ_IsKnownUser)
			return "#STR_EXPANSION_BB_CODE_UNLOCK";

		return "#STR_EXPANSION_BB_CODE_LOCK_ENTER_CODE";
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

		m_DevZ_IsKnownUser = m_DevZ_CodeLock.IsKnownUser(player);
		return true;
	}

	override void Start(ActionData action_data)
	{
		super.Start(action_data);

		if (!IsMissionClient() || !m_DevZ_CodeLock)
			return;

		if (m_DevZ_CodeLock.HasCode() && !m_DevZ_CodeLock.ExpansionIsLocked())
		{
			auto lockRpc = ExpansionScriptRPC.Create(ItemBase.s_Expansion_Lock_RPCID);
			lockRpc.Write("");
			lockRpc.Expansion_Send(m_DevZ_CodeLock, true);
			return;
		}

		if (m_DevZ_CodeLock.HasCode() && m_DevZ_IsKnownUser)
		{
			auto unlockRpc = ExpansionScriptRPC.Create(ItemBase.s_Expansion_Unlock_RPCID);
			unlockRpc.Write("");
			unlockRpc.Write("");
			unlockRpc.Expansion_Send(m_DevZ_CodeLock, true);
			return;
		}

		ExpansionCodeLockUI menu = ExpansionCodeLockUI.Cast(g_Game.GetUIManager().EnterScriptedMenu(MENU_EXPANSION_CODELOCK_MENU, null));
		if (menu)
		{
			menu.SetChangeCodelock(false);
			menu.SetConfirm(!m_DevZ_CodeLock.HasCode());
			menu.SetTarget(m_DevZ_CodeLock, "");
		}
	}
}
