// Vanilla points a placement preview the way the camera looks. The rack's model has its front
// the other way round, so it came out with its back to the player (barrels on a rack placed
// against a wall faced the wall). Turn the kit's preview, and so the placed rack, by 180
// degrees. Every other item keeps the vanilla facing.
modded class Hologram
{
	override vector GetDefaultOrientation()
	{
		vector orientation = super.GetDefaultOrientation();

		if (BarrelStorage_Kit.Cast(GetParentEntity()))
			orientation[0] = orientation[0] + 180;

		return orientation;
	}
};
