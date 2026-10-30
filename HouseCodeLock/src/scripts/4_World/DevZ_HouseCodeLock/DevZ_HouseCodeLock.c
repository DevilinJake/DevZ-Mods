// Lets the Expansion code lock go on rebuilt vanilla house doors, in the vanilla digital code
// lock's spots (Att_CodeLock1_Wood ... Att_CodeLock10_Metal, see config.cpp).
//
// Expansion's lock code expects the lock to sit on an item (fence, wall, tent, crate). A house is
// a building, not an item, so this mod points Expansion's own keypad and lock state at the lock
// itself (the code, the locked flag and the known players live on the lock anyway) and makes the
// house check that lock before a door opens. Nothing in Expansion's files is changed.
class DevZ_HouseCodeLock
{
	// How close to the lock the cursor must be for the lock actions when Expansion's
	// CodelockActionsAnywhere is off (metres from the lock; covers both keypads)
	static const float LOCK_ACTION_REACH = 0.4;

	// The door whose lock spots include slotName, or -1
	static int DoorIndexForSlot(Building house, string slotName)
	{
		map<int, ref AdditionalDoorInfo> doors = GetDoors(house);
		if (!doors)
			return -1;

		slotName.ToLower();
		foreach (int doorIndex, AdditionalDoorInfo door: doors)
		{
			if (!door || !door.m_RelatedInventorySlotNames)
				continue;

			foreach (string related: door.m_RelatedInventorySlotNames)
			{
				related.ToLower();
				if (related == slotName)
					return doorIndex;
			}
		}

		return -1;
	}

	// The Expansion code lock fitted to this door, or null
	static ExpansionCodeLock FindLockOnDoor(Building house, int doorIndex)
	{
		map<int, ref AdditionalDoorInfo> doors = GetDoors(house);
		if (!doors)
			return null;

		AdditionalDoorInfo door = doors.Get(doorIndex);
		if (!door || !door.m_RelatedInventorySlotNames)
			return null;

		foreach (string slotName: door.m_RelatedInventorySlotNames)
		{
			ExpansionCodeLock codeLock;
			if (Class.CastTo(codeLock, house.FindAttachmentBySlotName(slotName)))
				return codeLock;
		}

		return null;
	}

	// True if any lock (Expansion or vanilla) is fitted to this door in a spot other than slotName
	static bool DoorHasOtherLock(Building house, int doorIndex, string slotName)
	{
		map<int, ref AdditionalDoorInfo> doors = GetDoors(house);
		if (!doors)
			return false;

		AdditionalDoorInfo door = doors.Get(doorIndex);
		if (!door || !door.m_RelatedInventorySlotNames)
			return false;

		slotName.ToLower();
		foreach (string related: door.m_RelatedInventorySlotNames)
		{
			EntityAI fitted = house.FindAttachmentBySlotName(related);
			if (!fitted)
				continue;

			related.ToLower();
			if (related == slotName)
				continue;

			if (fitted.IsInherited(ExpansionCodeLock) || fitted.IsInherited(DigitalCodeLock) || fitted.IsInherited(CombinationLock))
				return true;
		}

		return false;
	}

	// What the player is looking at, as a house door: either the door itself (any part of it) or
	// the lock on it. Gives the house, the door and the Expansion lock on that door (null if none).
	// False when the target is neither.
	static bool ResolveDoor(ActionTarget target, out BuildingBase house, out int doorIndex, out ExpansionCodeLock codeLock)
	{
		house = null;
		doorIndex = -1;
		codeLock = null;

		if (!target)
			return false;

		if (Class.CastTo(house, target.GetObject()))
		{
			doorIndex = house.GetDoorIndex(target.GetComponentIndex());
			if (doorIndex < 0)
				return false;

			codeLock = FindLockOnDoor(house, doorIndex);
			return true;
		}

		return ResolveLockTarget(target, house, doorIndex, codeLock);
	}

	// Only when the cursor is on the lock itself (its hidden body in the door's slot)
	static bool ResolveLockTarget(ActionTarget target, out BuildingBase house, out int doorIndex, out ExpansionCodeLock codeLock)
	{
		house = null;
		doorIndex = -1;
		codeLock = null;

		if (!target || !Class.CastTo(codeLock, target.GetObject()))
			return false;

		house = codeLock.DevZ_GetHouse();
		if (!house)
		{
			codeLock = null;
			return false;
		}

		doorIndex = codeLock.DevZ_GetDoorIndex();
		return doorIndex > -1;
	}

	// Expansion's CodelockActionsAnywhere: on, the lock actions work from anywhere on the door;
	// off, only while looking at (near) one of the keypads, like the lock selection on a gate
	static bool LockActionsAllowed(ActionTarget target, ExpansionCodeLock codeLock)
	{
		if (!codeLock)
			return false;

		if (GetExpansionSettings().GetBaseBuilding().CodelockActionsAnywhere)
			return true;

		if (target.GetObject() == codeLock)
			return true;

		return vector.Distance(target.GetCursorHitPos(), codeLock.GetPosition()) <= LOCK_ACTION_REACH;
	}

	protected static map<int, ref AdditionalDoorInfo> GetDoors(Building house)
	{
		if (!house)
			return null;

		AdditionalDoorInfosHolder holder = house.GetDoorInfoHolder();
		if (!holder)
			return null;

		return holder.GetDoorInfoMap();
	}
}
