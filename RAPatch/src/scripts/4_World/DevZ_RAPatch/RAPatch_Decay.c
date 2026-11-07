// Decay and upkeep. RA reads all of this through its own virtual functions, so nothing of RA's has to be replaced.
//
// A part's upkeep must stay above 0 on the server: RA's ItemBase constructor uses "above 0" to decide whether to
// register a synced variable, and the client must make the same decision. 0.00001 is RA's own "as good as free".
class RAPatch_Decay
{
	static const float HOUR = 3600;
	static const float NEVER = 1000000000;	// seconds: a part loses next to nothing in any CE tick
	static const float SMALLEST_UPKEEP = 0.00001;

	static float Seconds(float hours)
	{
		RAPatch_DecaySettings decay = RAPatch_Settings.Get().Decay;
		if (!decay.Enabled || hours <= 0)
			return NEVER;
		return hours * HOUR;
	}

	static float Upkeep(float nailsPerDay)
	{
		return Math.Max(nailsPerDay, SMALLEST_UPKEEP);
	}
}

modded class ItemBase
{
	// Tents, barrels and crates are the only other things RA gives an upkeep. The desk and parts have their own.
	override float GetTimeToDecay()
	{
		return RAPatch_Decay.Seconds(RAPatch_Settings.Get().Decay.ContainerHours);
	}
}

modded class TentBase
{
	override float GetCostToUpkeep()
	{
		return RAPatch_Decay.Upkeep(RAPatch_Settings.Get().Decay.ContainerNailsPerDay);
	}
}

modded class DeployableContainer_Base
{
	override float GetCostToUpkeep()
	{
		return RAPatch_Decay.Upkeep(RAPatch_Settings.Get().Decay.ContainerNailsPerDay);
	}
}

modded class BaseBuilding_StepLadderBase
{
	override float GetCostToUpkeep()
	{
		return RAPatch_Decay.Upkeep(RAPatch_Settings.Get().Decay.LadderNailsPerDay);
	}
}
