// The placed rack: holds up to 6 barrels as attachments. It can't be picked up or put in
// cargo; dismantle it with a screwdriver (empty) to get the kit back.
class Invictuz_Barrel_Storage extends ItemBase
{
	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		return false;
	}

	override bool CanReleaseCargo(EntityAI cargo)
	{
		return false;
	}
};

// The placement preview of the kit (set by projectionTypename in config.cpp)
class BarrelStorage_Holo extends Invictuz_Barrel_Storage {};
