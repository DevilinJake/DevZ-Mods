// Hooks every vehicle type into DevZ Crash Safety:
// - Crew: replaces crash damage to the crew of cars and motorbikes. Vanilla kills the crew above a
//   set impact; here a crash only takes health (never below 1) and knocks the crew out for a while.
//   Leave other mods' crew damage settings (e.g. Expansion's VehicleCrewDamageMultiplier) at 1:
//   depending on load order they may or may not apply on top of this.
// - Floors: damage events go to DevZ_CrashSafetyFloors (3_Game). The three vehicle bases each pass
//   them on because their common base, Transport, can't be modded.

class DevZ_CrashSafetyCrew
{
	static void Apply(Transport vehicle, float dmg, bool isBike)
	{
		float severity = DevZ_CrashSafetyConfig.Get().Severity(dmg, isBike);
		for (int c = 0; c < vehicle.CrewSize(); ++c)
		{
			PlayerBase player = PlayerBase.Cast(vehicle.CrewMember(c));
			if (player)
				player.DevZ_CS_ApplyCrash(severity, vehicle.GetType());
		}
	}
}

// Cars, trucks, and Expansion's helicopters and boats (they are CarScripts too)
modded class CarScript
{
	override void DamageCrew(float dmg)
	{
		if (!DevZ_CrashSafetyConfig.Get().Enabled)
		{
			super.DamageCrew(dmg);
			return;
		}
		DevZ_CrashSafetyCrew.Apply(this, dmg, false);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_CrashSafetyFloors.OnDamage(this, damageResult, damageType);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		DevZ_CrashSafetyFloors.EndAllowRuin();
	}

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		DevZ_CrashSafetyFloors.OnLevelChanged(this, newLevel, zone);
	}
}

modded class MotorbikeScript
{
	override void DamageCrew(float dmg)
	{
		if (!DevZ_CrashSafetyConfig.Get().Enabled)
		{
			super.DamageCrew(dmg);
			return;
		}
		DevZ_CrashSafetyCrew.Apply(this, dmg, true);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_CrashSafetyFloors.OnDamage(this, damageResult, damageType);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		DevZ_CrashSafetyFloors.EndAllowRuin();
	}

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		DevZ_CrashSafetyFloors.OnLevelChanged(this, newLevel, zone);
	}
}

// Vanilla boats
modded class BoatScript
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_CrashSafetyFloors.OnDamage(this, damageResult, damageType);
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		DevZ_CrashSafetyFloors.EndAllowRuin();
	}

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		DevZ_CrashSafetyFloors.OnLevelChanged(this, newLevel, zone);
	}

	// Boats rot while no player is near, so abandoned ones end up ruined and the loot economy
	// respawns them elsewhere. Decay is left free to ruin them.
	override protected void DecayHealthTick()
	{
		DevZ_CrashSafetyFloors.AllowRuin(this);
		super.DecayHealthTick();
		DevZ_CrashSafetyFloors.EndAllowRuin();
	}
}
