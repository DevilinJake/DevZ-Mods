// Vanilla "Close" on house doors: when Expansion's CodelockActionsAnywhere is off (its default),
// closing a door whose lock has a code reads "Lock" and locks it, the same as Expansion's own
// "Close" on gates and doors. With it on, closing just closes and "Lock" is a separate action.
modded class ActionCloseDoors
{
	protected bool m_DevZ_Lock;

	override string GetText()
	{
		if (m_DevZ_Lock)
			return "#STR_EXPANSION_BB_CODE_CLOSE_LOCK";

		return super.GetText();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_DevZ_Lock = false;

		if (!super.ActionCondition(player, target, item))
			return false;

		m_DevZ_Lock = DevZ_LocksOnClose(target);
		return true;
	}

	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		if (!DevZ_LocksOnClose(action_data.m_Target))
			return;

		BuildingBase house;
		int doorIndex;
		ExpansionCodeLock codeLock;
		DevZ_HouseCodeLock.ResolveDoor(action_data.m_Target, house, doorIndex, codeLock);
		codeLock.ExpansionLock();
	}

	protected bool DevZ_LocksOnClose(ActionTarget target)
	{
		if (!GetExpansionSettings().GetBaseBuilding(false).IsLoaded() || GetExpansionSettings().GetBaseBuilding().CodelockActionsAnywhere)
			return false;

		BuildingBase house;
		int doorIndex;
		ExpansionCodeLock codeLock;
		if (!DevZ_HouseCodeLock.ResolveDoor(target, house, doorIndex, codeLock) || !codeLock)
			return false;

		return codeLock.HasCode() && !codeLock.ExpansionIsLocked() && !codeLock.IsRuined();
	}
}
