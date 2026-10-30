modded class BuildingBase
{
	// A door with a locked Expansion code lock stays shut (checked on the player's side and again
	// on the server). A ruined lock no longer holds it, like the vanilla locks.
	override bool CanDoorBeOpened(notnull DoorManipulationParams params)
	{
		if (!super.CanDoorBeOpened(params))
			return false;

		ExpansionCodeLock codeLock = DevZ_HouseCodeLock.FindLockOnDoor(this, params.m_DoorIndex);
		if (codeLock && codeLock.ExpansionIsLocked() && !codeLock.IsRuined())
			return false;

		return true;
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
