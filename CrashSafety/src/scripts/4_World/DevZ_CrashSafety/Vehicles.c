// Crew: replaces crash damage to the crew of cars and motorbikes. Vanilla kills the crew above a
// set impact; here a crash only takes health (never below 1) and knocks the crew out for a while.
// Leave other mods' crew damage settings (e.g. Expansion's VehicleCrewDamageMultiplier) at 1:
// depending on load order they may or may not apply on top of this.
//
// Hull: crashes, falls and fire stop at HullFloorPercent of the vehicle's overall health, so it
// ends up Badly Damaged instead of Ruined (a ruined vehicle burns out and can't be repaired).
// Damage zones and attached parts still take every hit in full.

class DevZ_CrashSafetyHull
{
	// Runs right before the game applies a hit to a vehicle. Returns false to cancel the hit.
	static bool OnDamage(EntityAI vehicle, TotalDamageResult damageResult, int damageType)
	{
		if (!g_Game.IsServer() || !damageResult)
			return true;

		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();
		if (!cfg.Enabled || cfg.HullFloorPercent <= 0)
			return true;
		// Crashes, falls and fire are CUSTOM; guns, explosives and melee only if set
		if (damageType != DamageType.CUSTOM && !cfg.HullFloorAgainstWeapons)
			return true;

		float health = vehicle.GetHealth("", "Health");
		if (health <= 0)
			return true;

		float maxHealth = vehicle.GetMaxHealth("", "Health");
		float dmg = damageResult.GetDamage("", "Health");
		// Already below the floor (e.g. shot up in PvP): the hull keeps what it has
		float floor = Math.Min(maxHealth * cfg.HullFloorPercent * 0.01, health);
		if (health - dmg >= floor)
			return true;

		// Top the hull up by the overshoot so the hit lands exactly on the floor, and let the hit
		// go through as normal so zones and attached parts take their share
		float topUp = floor - (health - dmg);
		if (health + topUp <= maxHealth)
		{
			vehicle.AddHealth("", "Health", topUp);
			Log(cfg, vehicle, health, floor);
			return true;
		}

		// A single hit bigger than the whole hull: cancel it, apply it to each zone by hand and
		// set the hull to the floor
		TStringArray zones = new TStringArray();
		vehicle.GetDamageZones(zones);
		foreach (string zone : zones)
		{
			float zoneDmg = damageResult.GetDamage(zone, "Health");
			if (zoneDmg > 0)
				vehicle.DecreaseHealth(zone, "Health", zoneDmg, false);
		}
		vehicle.SetHealth("", "Health", floor);
		Log(cfg, vehicle, health, floor);
		return false;
	}

	protected static void Log(DevZ_CrashSafetyConfig cfg, EntityAI vehicle, float before, float floor)
	{
		if (cfg.LogCrashes)
			PrintFormat("[DevZ_CrashSafety] %1 hull held at %2 (was %3, max %4)", vehicle.GetType(), floor, before, vehicle.GetMaxHealth("", "Health"));
	}

	static void DamageCrew(Transport vehicle, float dmg, bool isBike)
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
		DevZ_CrashSafetyHull.DamageCrew(this, dmg, false);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_CrashSafetyHull.OnDamage(this, damageResult, damageType);
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
		DevZ_CrashSafetyHull.DamageCrew(this, dmg, true);
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_CrashSafetyHull.OnDamage(this, damageResult, damageType);
	}
}

// Vanilla boats
modded class BoatScript
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_CrashSafetyHull.OnDamage(this, damageResult, damageType);
	}
}
