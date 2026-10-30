// The kit works like vanilla base-building kits (shelter, fence): it stays in the player's
// hands while deploying, spawns the rack where the preview was, hides itself, and the deploy
// action deletes it when it ends. Deleting it in OnPlacementComplete (the old way) broke on
// 1.30, where the action moves the item to the ground right before that call.
class BarrelStorage_Kit extends ItemBase
{
	override bool IsBasebuildingKit()
	{
		return true;
	}

	override bool IsDeployable()
	{
		return true;
	}

	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionTogglePlaceObject);
		AddAction(ActionDeployObject);
	}

	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		super.OnPlacementComplete(player, position, orientation);

		if (g_Game.IsServer())
		{
			EntityAI rack = EntityAI.Cast(g_Game.CreateObjectEx("Invictuz_Barrel_Storage", position, ECE_PLACE_ON_SURFACE));
			if (rack)
			{
				rack.SetPosition(position);
				rack.SetOrientation(orientation);
			}

			HideAllSelections();
		}
	}

	override string GetDeploySoundset()
	{
		return "putDown_FenceKit_SoundSet";
	}

	override string GetLoopDeploySoundset()
	{
		return "BarbedWire_Deploy_loop_SoundSet";
	}
};
