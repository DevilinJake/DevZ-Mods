// Code lock in hands, looking at a closed, rebuilt house door: "Attach". Uses the vanilla attach
// action, limited to house doors (Expansion's own attach action still handles gates, walls and
// tents). With Expansion's "open lock menu after placing" client setting on, the keypad opens
// right away, as it does on a gate.
class DevZ_ActionAttachCodeLockToHouse: ActionAttachToConstruction
{
	override protected int FindAndTranslateAttachmentSlot(PlayerBase player, ActionTarget target, ItemBase item)
	{
		BuildingBase house;
		if (!Class.CastTo(house, target.GetObject()) || !house.CanUseConstruction())
			return InventorySlots.INVALID;

		int doorIndex = house.GetDoorIndex(target.GetComponentIndex());
		if (doorIndex < 0 || house.IsDoorLocked(doorIndex) || house.IsDoorOpen(doorIndex))
			return InventorySlots.INVALID;

		// One code lock per door
		if (DevZ_HouseCodeLock.FindLockOnDoor(house, doorIndex))
			return InventorySlots.INVALID;

		AdditionalDoorInfosHolder holder = house.GetDoorInfoHolder();
		if (!holder || !holder.GetDoorInfoMap())
			return InventorySlots.INVALID;

		AdditionalDoorInfo door = holder.GetDoorInfoMap().Get(doorIndex);
		if (!door || !door.m_RelatedInventorySlotNames)
			return InventorySlots.INVALID;

		foreach (string slotName: door.m_RelatedInventorySlotNames)
		{
			if (house.FindAttachmentBySlotName(slotName))
				continue;

			int slotId = InventorySlots.GetSlotIdFromString(slotName);
			if (slotId != InventorySlots.INVALID && house.GetInventory().CanAddAttachmentEx(item, slotId))
				return slotId;
		}

		return InventorySlots.INVALID;
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);

		if (!GetExpansionClientSettings().AutoOpenLockMenuAfterPlacing)
			return;

		ExpansionCodeLock codeLock = ExpansionCodeLock.Cast(action_data.m_MainItem);
		if (codeLock)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DevZ_OpenKeypad, 250, false, codeLock);
	}

	void DevZ_OpenKeypad(ExpansionCodeLock codeLock)
	{
		if (!codeLock || !codeLock.DevZ_GetHouse() || codeLock.HasCode())
			return;

		ExpansionCodeLockUI menu = ExpansionCodeLockUI.Cast(g_Game.GetUIManager().EnterScriptedMenu(MENU_EXPANSION_CODELOCK_MENU, null));
		if (menu)
		{
			menu.SetChangeCodelock(false);
			menu.SetConfirm(true);
			menu.SetTarget(codeLock, "");
		}
	}
}
