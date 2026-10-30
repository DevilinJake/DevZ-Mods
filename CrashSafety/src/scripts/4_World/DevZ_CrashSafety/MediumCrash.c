// Vanilla sorts a car crash into three bands by its damage: under 150 nothing happens; 150 to 750
// (medium) damages only the body zone that was hit; from 750 (heavy) the damage also passes on to
// the parts linked to that zone (radiator, battery, lights...) and hurts the crew. So parts go
// straight from fine to ruined. MediumCrashPartsPercent passes a share of a medium crash on to the
// linked parts as well, so they wear down over a few knocks instead.

class DevZ_CrashSafetyMediumCrash
{
	static void Apply(CarScript vehicle, VehicleContactCache cache, float dmgContactCoef)
	{
		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();
		if (!cfg.Enabled)
			return;

		for (int i = 0; i < cache.Count(); ++i)
		{
			string zone = cache.GetKey(i);
			array<ref VehicleContactData> data = cache.GetElement(i);
			if (!data || data.Count() == 0)
				continue;

			// Same sums as vanilla CarScript.CheckContactCache
			float dmg = Math.AbsInt(data[0].m_Impulse * dmgContactCoef);
			float crewDmg = Math.AbsInt((data[0].m_Impulse / dBodyGetMass(vehicle)) * 1000 * dmgContactCoef);
			if (dmg < GameConstants.CARS_CONTACT_DMG_MIN)
				continue;

			bool medium = dmg < GameConstants.CARS_CONTACT_DMG_THRESHOLD;
			// Helicopters report crashes with no zone; leave them to Expansion
			if (medium && zone != "" && cfg.MediumCrashPartsPercent > 0)
				PassOn(vehicle, zone, dmg * cfg.MediumCrashPartsPercent * 0.01);

			if (cfg.LogCrashes)
			{
				string band = "heavy";
				if (medium)
					band = "medium";
				PrintFormat("[DevZ_CrashSafety] %1 %2 crash on %3: damage %4 (crew %5), %6 km/h after", vehicle.GetType(), band, zone, dmg, crewDmg, Math.Round(GetVelocity(vehicle).Length() * 3.6));
			}
		}
	}

	// Damage the parts linked to a zone without hitting the zone itself a second time: vanilla's
	// own medium hit still lands on the zone afterwards.
	protected static void PassOn(CarScript vehicle, string zone, float amount)
	{
		float zoneHealth = vehicle.GetHealth(zone, "Health");
		// A zone this close to ruined would be ruined by the pass-on before it could be put back
		if (zoneHealth <= amount)
			return;
		vehicle.ProcessDirectDamage(DamageType.CUSTOM, null, zone, "EnviroDmg", "0 0 0", amount);
		if (vehicle.GetHealth(zone, "Health") < zoneHealth)
			vehicle.SetHealth(zone, "Health", zoneHealth);
	}
}

modded class CarScript
{
	override void CheckContactCache()
	{
		if (g_Game.IsServer() && m_ContactCache)
			DevZ_CrashSafetyMediumCrash.Apply(this, m_ContactCache, m_dmgContactCoef);
		super.CheckContactCache();
	}
}
