modded class BuildingBase
{
	// A door with a locked Expansion code lock stays shut, except for a player who knows the code
	// (entered it before, or a territory member when Expansion is set up that way): they get
	// "Open locked door" straight away, like on an Expansion gate. Checked on the player's side
	// and again on the server. A ruined lock no longer holds the door, like the vanilla locks.
	override bool CanDoorBeOpened(notnull DoorManipulationParams params)
	{
		if (!super.CanDoorBeOpened(params))
			return false;

		ExpansionCodeLock codeLock = DevZ_HouseCodeLock.FindLockOnDoor(this, params.m_DoorIndex);
		if (codeLock && codeLock.DevZ_HoldsDoor())
		{
			PlayerBase player = PlayerBase.Cast(params.m_Caller);
			return player && codeLock.IsKnownUser(player);
		}

		return true;
	}

	// Opening a locked door unlocks it (the same as opening a locked Expansion gate): the light
	// goes red and anyone can use the door until it's locked again
	override void OnDoorOpenStart(DoorStartParams params)
	{
		super.OnDoorOpenStart(params);

		if (!g_Game.IsServer())
			return;

		ExpansionCodeLock codeLock = DevZ_HouseCodeLock.FindLockOnDoor(this, params.param1);
		if (codeLock && codeLock.ExpansionIsLocked())
			codeLock.ExpansionUnlock();
	}

	// Vanilla only applies its door-lock rules to its own locks; apply them to the Expansion lock
	// too: one lock per door (also against vanilla locks, either way round), and only while that
	// door is shut
	override bool CanReceiveAttachment(EntityAI attachment, int slotId)
	{
		if (!super.CanReceiveAttachment(attachment, slotId))
			return false;

		if (!attachment)
			return true;

		bool isExpansionLock = attachment.IsInherited(ExpansionCodeLock);
		bool isVanillaLock = attachment.IsInherited(DigitalCodeLock) || attachment.IsInherited(CombinationLock);
		if (!isExpansionLock && !isVanillaLock)
			return true;

		string slotName = InventorySlots.GetSlotName(slotId);
		int doorIndex = DevZ_HouseCodeLock.DoorIndexForSlot(this, slotName);

		// A vanilla lock can't go on a door that already has an Expansion lock
		if (isVanillaLock)
			return doorIndex < 0 || !DevZ_HouseCodeLock.FindLockOnDoor(this, doorIndex);

		if (HasConflictingDoorAttachment(slotName, ATTACHMENT_SLOT_CODE_LOCK, ATTACHMENT_SLOT_COMBINATION_LOCK))
			return false;

		if (HasConflictingDoorAttachment(slotName, ATTACHMENT_SLOT_COMBINATION_LOCK, ATTACHMENT_SLOT_CODE_LOCK))
			return false;

		if (doorIndex > -1)
		{
			if (IsDoorOpen(doorIndex))
				return false;

			if (DevZ_HouseCodeLock.DoorHasOtherLock(this, doorIndex, slotName))
				return false;
		}

		return true;
	}
}
