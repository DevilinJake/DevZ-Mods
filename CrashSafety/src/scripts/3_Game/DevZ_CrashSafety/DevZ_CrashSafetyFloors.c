// Floors for the parts that would end a vehicle for good: the hull, engine and fuel tank (see
// DevZ_CrashSafetyConfig.FloorPercent). Cars, motorbikes and boats pass their damage events here
// (4_World\DevZ_CrashSafety\Vehicles.c); their shared base, Transport, is an engine class and
// can't be modded.
//
// Two hooks, one rule:
// - Before a hit lands (crashes, falls, fire, rev damage), each held part is topped up by just
//   enough that the hit leaves it on its floor. The hit itself goes through unchanged, so the other
//   zones and attached parts still take their share. This is the only way to save the hull: once
//   it reaches 0 the vehicle is destroyed and burns.
// - Some damage skips the damage system and sets health directly (over-revving, no coolant, deep
//   water, "fuel tank ruined so engine ruined"). If that ruins the engine or fuel tank, it's put
//   straight back on its floor.
// Damage that should still ruin parts runs inside AllowRuin: weapon hits (unless
// FloorsAgainstWeapons is set) and boat decay, so abandoned boats still rot away and respawn
// elsewhere.

class DevZ_CrashSafetyFloors
{
	// The vehicle whose current damage may ruin parts. Damage is applied one hit at a time, so one
	// slot is enough.
	protected static EntityAI s_AllowRuin;

	static void AllowRuin(EntityAI vehicle)
	{
		s_AllowRuin = vehicle;
	}

	static void EndAllowRuin()
	{
		s_AllowRuin = null;
	}

	// EEOnDamageCalculated: returns false if the hit was cancelled (and applied by hand instead)
	static bool OnDamage(EntityAI vehicle, TotalDamageResult damageResult, int damageType)
	{
		EndAllowRuin();
		if (!g_Game.IsServer() || !damageResult)
			return true;

		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();
		if (!cfg.Enabled || vehicle.IsDamageDestroyed())
			return true;
		// Crashes, falls, fire and engine damage are CUSTOM; guns, explosives and melee are not
		if (damageType != DamageType.CUSTOM && !cfg.FloorsAgainstWeapons)
		{
			AllowRuin(vehicle);
			return true;
		}

		return HoldParts(cfg, vehicle, damageResult);
	}

	// EEHealthLevelChanged: catches damage that skipped OnDamage
	static void OnLevelChanged(EntityAI vehicle, int newLevel, string zone)
	{
		// The hull isn't handled here: a ruined hull is a destroyed vehicle and can't be brought back
		if (newLevel != GameConstants.STATE_RUINED || zone == "" || s_AllowRuin == vehicle)
			return;
		if (!g_Game.IsServer() || vehicle.IsDamageDestroyed())
			return;

		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();
		float floor = vehicle.GetMaxHealth(zone, "Health") * cfg.FloorPercent(zone) * 0.01;
		if (!cfg.Enabled || floor <= 0)
			return;

		vehicle.SetHealth(zone, "Health", floor);
		Log(cfg, vehicle, zone, "kept from ruin", floor);
	}

	// True if a held part is within margin of its floor (or below it). For damage that skips the
	// damage system, so it can be held off before it lands.
	static bool NearFloor(EntityAI vehicle, string zone, float margin)
	{
		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();
		if (!cfg.Enabled)
			return false;
		float floor = vehicle.GetMaxHealth(zone, "Health") * cfg.FloorPercent(zone) * 0.01;
		return floor > 0 && vehicle.GetHealth(zone, "Health") - margin < floor;
	}

	// Tops up each held part so the coming hit leaves it on its floor. Returns false if the hit had
	// to be cancelled and applied by hand instead.
	protected static bool HoldParts(DevZ_CrashSafetyConfig cfg, EntityAI vehicle, TotalDamageResult damageResult)
	{
		float hull = TopUp(cfg, vehicle, damageResult, "");
		float engine = TopUp(cfg, vehicle, damageResult, "Engine");
		float fuelTank = TopUp(cfg, vehicle, damageResult, "FuelTank");
		if (hull == 0 && engine == 0 && fuelTank == 0)
			return true;

		if (Fits(vehicle, "", hull) && Fits(vehicle, "Engine", engine) && Fits(vehicle, "FuelTank", fuelTank))
		{
			AddTopUp(cfg, vehicle, "", hull);
			AddTopUp(cfg, vehicle, "Engine", engine);
			AddTopUp(cfg, vehicle, "FuelTank", fuelTank);
			return true;
		}

		// A single hit bigger than a held part's whole health can't be topped up: apply it to every
		// zone by hand, stopping held parts on their floors, and cancel the original
		TStringArray zones = new TStringArray();
		vehicle.GetDamageZones(zones);
		zones.Insert("");
		foreach (string zone : zones)
		{
			float dmg = damageResult.GetDamage(zone, "Health");
			if (dmg <= 0)
				continue;
			float after = vehicle.GetHealth(zone, "Health") - dmg;
			float floor = Floor(cfg, vehicle, zone);
			vehicle.SetHealth(zone, "Health", Math.Max(after, floor));
			if (after < floor)
				Log(cfg, vehicle, zone, "held on its floor", floor);
		}
		return false;
	}

	// A part's floor in health points (0 if it isn't held). A part already below it keeps what it has.
	protected static float Floor(DevZ_CrashSafetyConfig cfg, EntityAI vehicle, string zone)
	{
		float floor = vehicle.GetMaxHealth(zone, "Health") * cfg.FloorPercent(zone) * 0.01;
		return Math.Min(floor, vehicle.GetHealth(zone, "Health"));
	}

	// Health a part needs added so the hit leaves it on its floor (0 if it stays above, isn't held,
	// or the vehicle has no such part)
	protected static float TopUp(DevZ_CrashSafetyConfig cfg, EntityAI vehicle, TotalDamageResult damageResult, string zone)
	{
		float floor = Floor(cfg, vehicle, zone);
		if (floor <= 0)
			return 0;
		float after = vehicle.GetHealth(zone, "Health") - damageResult.GetDamage(zone, "Health");
		return Math.Max(floor - after, 0);
	}

	protected static bool Fits(EntityAI vehicle, string zone, float topUp)
	{
		return vehicle.GetHealth(zone, "Health") + topUp <= vehicle.GetMaxHealth(zone, "Health");
	}

	protected static void AddTopUp(DevZ_CrashSafetyConfig cfg, EntityAI vehicle, string zone, float topUp)
	{
		if (topUp <= 0)
			return;
		Log(cfg, vehicle, zone, "held on its floor", Floor(cfg, vehicle, zone));
		vehicle.AddHealth(zone, "Health", topUp);
	}

	protected static void Log(DevZ_CrashSafetyConfig cfg, EntityAI vehicle, string zone, string what, float health)
	{
		if (!cfg.LogCrashes)
			return;
		string part = zone;
		if (part == "")
			part = "hull";
		PrintFormat("[DevZ_CrashSafety] %1 %2 %3 (health %4 of %5)", vehicle.GetType(), part, what, health, vehicle.GetMaxHealth(zone, "Health"));
	}
}
