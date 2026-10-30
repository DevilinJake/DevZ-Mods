modded class ExpansionCodeLock
{
	// The two keypads players see on a house door (clients only, see DevZ_HouseCodeLockVisual)
	protected DevZ_HouseCodeLockVisual m_DevZ_KeypadOutside;
	protected DevZ_HouseCodeLockVisual m_DevZ_KeypadInside;

	// The house this lock is fitted to, or null when it's on an item (gate, wall, tent...) or loose
	BuildingBase DevZ_GetHouse()
	{
		if (!GetInventory() || !GetInventory().IsAttachment())
			return null;

		return BuildingBase.Cast(GetHierarchyParent());
	}

	// The house door this lock is on, or -1
	int DevZ_GetDoorIndex()
	{
		BuildingBase house = DevZ_GetHouse();
		if (!house)
			return -1;

		InventoryLocation location = new InventoryLocation();
		GetInventory().GetCurrentInventoryLocation(location);
		return DevZ_HouseCodeLock.DoorIndexForSlot(house, InventorySlots.GetSlotName(location.GetSlot()));
	}

	// Locked and able to hold the door (a ruined lock no longer does, like the vanilla locks)
	bool DevZ_HoldsDoor()
	{
		return ExpansionIsLocked() && !IsRuined();
	}

	override void SetActions()
	{
		super.SetActions();

		// Lock in hands, looking at a door. The Set code / Lock / Enter code / Unlock / Change code
		// actions are the player's (PlayerBase.c): they work on the door and on the lock alike.
		AddAction(DevZ_ActionAttachCodeLockToHouse);
	}

	// Counts as a door lock, like the vanilla locks: a lockpick can't lock or unlock a door with
	// one fitted
	override bool IsExternalLockType()
	{
		if (DevZ_GetHouse())
			return true;

		return super.IsExternalLockType();
	}

	// On a house the lock is "open" while its door is. Expansion uses this to decide whether
	// setting a code locks straight away (door shut) or waits for "Lock" (door open), the same as
	// on a gate.
	override bool IsOpen()
	{
		int doorIndex = DevZ_GetDoorIndex();
		if (doorIndex > -1)
			return DevZ_GetHouse().IsDoorOpen(doorIndex);

		return super.IsOpen();
	}

	// "Lock" with the door open shuts the house door first, then Expansion locks the lock
	override void Expansion_Close(string selection)
	{
		int doorIndex = DevZ_GetDoorIndex();
		if (doorIndex > -1)
		{
			BuildingBase house = DevZ_GetHouse();
			if (house.IsDoorOpen(doorIndex))
				house.CloseDoor(doorIndex);
			return;
		}

		super.Expansion_Close(selection);
	}

	// Expansion remembers a player who enters the right code by asking the lock's parent for its
	// lock, which finds nothing when the keypad talks to the lock itself. Remember them here.
	override void RPC_Expansion_Unlock(PlayerIdentity sender, ParamsReadContext ctx)
	{
		bool wasLocked = ExpansionIsLocked();

		super.RPC_Expansion_Unlock(sender, ctx);

		if (!DevZ_GetHouse() || !wasLocked || ExpansionIsLocked() || !sender)
			return;

		if (GetExpansionSettings().GetBaseBuilding().RememberCode)
			AddUser(PlayerBase.Cast(sender.GetPlayer()));
	}

	// After a restart Expansion clears the code of any lock that isn't on an item, which would
	// unlock every house door. Put the code, locked state and known players back.
	override void DeferredInit()
	{
		BuildingBase house = DevZ_GetHouse();
		if (!house || !g_Game.IsServer() || !HasCode())
		{
			super.DeferredInit();
			return;
		}

		string code = m_Expansion_Code;
		bool locked = m_Expansion_Locked;
		TStringArray knownUIDs = new TStringArray();
		knownUIDs.Copy(m_Expansion_KnownUIDs);

		super.DeferredInit();

		m_Expansion_Code = code;
		m_Expansion_CodeLength = code.Length();
		m_Expansion_Locked = locked;
		m_Expansion_KnownUIDs.Copy(knownUIDs);
		SetSlotLock(house, true);
		SetTakeable(false);
		SetSynchDirty();
	}

	override void OnWasDetached(EntityAI parent, int slot_id)
	{
		DevZ_DeleteKeypads();

		super.OnWasDetached(parent, slot_id);
	}

	override void EEDelete(EntityAI parent)
	{
		DevZ_DeleteKeypads();

		super.EEDelete(parent);
	}

	// On a house the lock itself is hidden (the door's slot points its model into the door) and
	// drawn as two keypads instead, one on each side of the door. Their light shows the lock
	// state: red unlocked, green locked.
	override protected void UpdateVisuals_Deferred()
	{
		if (!DevZ_GetHouse())
		{
			DevZ_DeleteKeypads();
			super.UpdateVisuals_Deferred();
			return;
		}

		HideSelection("camo");
		HideSelection("Codelock");
		HideSelection("attach_fence");

		if (g_Game.IsDedicatedServer())
			return;

		if (!m_DevZ_KeypadOutside || !m_DevZ_KeypadInside)
			DevZ_CreateKeypads();

		bool locked = ExpansionIsLocked();
		if (m_DevZ_KeypadOutside)
			m_DevZ_KeypadOutside.DevZ_SetLight(locked);
		if (m_DevZ_KeypadInside)
			m_DevZ_KeypadInside.DevZ_SetLight(locked);
	}

	// Places the two keypads as children of this lock (so they move with it, and with the door).
	// The lock model in the door's slot sits flush on the outside of the door with its keypad
	// facing INTO the door, so the outside keypad is that model turned 180 degrees about the
	// vertical through its own centre, and the inside keypad is the model as it is, moved through
	// the door onto the inside face. The door's slot always faces the lock the same way (its
	// forward axis points out of the door), so that axis gives the direction through the door
	// whatever the door is doing; the distances come from config.cpp.
	protected void DevZ_CreateKeypads()
	{
		DevZ_DeleteKeypads();

		vector lockMat[4];
		GetTransform(lockMat);

		vector centre = g_Game.ObjectGetSelectionPositionLS(this, "Codelock");
		centre = centre.Multiply4(lockMat);

		// outwardNormal points from the door towards the lock (the outside)
		string dirText = g_Game.ConfigGetTextOut("CfgVehicles DevZ_HouseCodeLockVisual devz_outwardDir");
		vector localDir = dirText.ToVector();
		if (localDir == vector.Zero)
			localDir = "0 0 1";
		vector outwardNormal = lockMat[0] * localDir[0] + lockMat[1] * localDir[1] + lockMat[2] * localDir[2];
		outwardNormal = outwardNormal.Normalized();

		float halfDepth = g_Game.ConfigGetFloat("CfgVehicles DevZ_HouseCodeLockVisual devz_lockDepth") * 0.5;
		float doorThickness = g_Game.ConfigGetFloat("CfgVehicles DevZ_HouseCodeLockVisual devz_doorThickness");
		vector raise = Vector(0, g_Game.ConfigGetFloat("CfgVehicles DevZ_HouseCodeLockVisual devz_raise"), 0);

		Print("[DevZ_HouseCodeLock] keypads: lockPos=" + lockMat[3] + " dir=" + lockMat[2] + " centre=" + centre + " outwardNormal=" + outwardNormal + " doorThickness=" + doorThickness + " halfDepth=" + halfDepth);

		// Outside: the lock turned 180 degrees about the vertical through its centre
		vector outsideMat[4];
		outsideMat[0] = DevZ_TurnAround(lockMat[0]);
		outsideMat[1] = DevZ_TurnAround(lockMat[1]);
		outsideMat[2] = DevZ_TurnAround(lockMat[2]);
		outsideMat[3] = centre + DevZ_TurnAround(lockMat[3] - centre) + raise;
		m_DevZ_KeypadOutside = DevZ_SpawnKeypad(lockMat, outsideMat);

		// Inside: the lock as it is, moved through the door onto its inside face
		vector insideMat[4];
		insideMat[0] = lockMat[0];
		insideMat[1] = lockMat[1];
		insideMat[2] = lockMat[2];
		insideMat[3] = lockMat[3] - outwardNormal * (2 * halfDepth + doorThickness) + raise;
		m_DevZ_KeypadInside = DevZ_SpawnKeypad(lockMat, insideMat);
	}

	// The vector turned 180 degrees about the vertical
	protected static vector DevZ_TurnAround(vector v)
	{
		return Vector(-v[0], v[1], -v[2]);
	}

	protected DevZ_HouseCodeLockVisual DevZ_SpawnKeypad(vector lockMat[4], vector worldMat[4])
	{
		DevZ_HouseCodeLockVisual keypad;
		if (!Class.CastTo(keypad, g_Game.CreateObjectEx("DevZ_HouseCodeLockVisual", worldMat[3], ECE_LOCAL | ECE_KEEPHEIGHT | ECE_NOLIFETIME)))
			return null;

		// Child transforms are relative to the parent (same as vanilla does for arrows stuck in things)
		vector localMat[4];
		Math3D.MatrixInvMultiply4(lockMat, worldMat, localMat);
		Math3D.MatrixOrthogonalize4(localMat);
		keypad.SetTransform(localMat);
		AddChild(keypad, -1);

		return keypad;
	}

	protected void DevZ_DeleteKeypads()
	{
		if (m_DevZ_KeypadOutside)
		{
			RemoveChild(m_DevZ_KeypadOutside, false);
			g_Game.ObjectDelete(m_DevZ_KeypadOutside);
			m_DevZ_KeypadOutside = null;
		}

		if (m_DevZ_KeypadInside)
		{
			RemoveChild(m_DevZ_KeypadInside, false);
			g_Game.ObjectDelete(m_DevZ_KeypadInside);
			m_DevZ_KeypadInside = null;
		}

	}
}
