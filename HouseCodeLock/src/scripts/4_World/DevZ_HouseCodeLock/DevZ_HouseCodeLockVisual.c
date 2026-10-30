// The keypad players see on a house door (one each side). Client-side only: the real
// ExpansionCodeLock sits hidden in the door's slot and does all the work. This object has no
// actions, can't be picked up, doesn't show in the vicinity and the cursor looks straight through
// it to the door behind (IsHologram), so every interaction lands on the door.
class DevZ_HouseCodeLockVisual: ItemBase
{
	static const string MAT_DEFAULT = "DayZExpansion/Objects/Basebuilding/Items/Codelock/data/codelock.rvmat";
	static const string MAT_RED = "dz/gear/camping/data/battery_charger_light_r.rvmat";
	static const string MAT_GREEN = "dz/gear/camping/data/battery_charger_light_g.rvmat";

	override void SetActions()
	{
		// None: not even Take or Inspect
	}

	override bool IsHologram()
	{
		return true;
	}

	override bool IsInventoryVisible()
	{
		return false;
	}

	override bool IsTakeable()
	{
		return false;
	}

	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanPutAsAttachment(EntityAI parent)
	{
		return false;
	}

	override bool CanBeActionTarget()
	{
		return false;
	}

	override bool CanDisplayCargo()
	{
		return false;
	}

	// Red when unlocked, green when locked (the same materials Expansion uses on the lock itself)
	void DevZ_SetLight(bool locked)
	{
		if (locked)
		{
			SetObjectMaterial(0, MAT_DEFAULT);
			SetObjectMaterial(1, MAT_GREEN);
		}
		else
		{
			SetObjectMaterial(0, MAT_RED);
			SetObjectMaterial(1, MAT_DEFAULT);
		}

		SetObjectMaterial(2, MAT_DEFAULT);
	}
}
