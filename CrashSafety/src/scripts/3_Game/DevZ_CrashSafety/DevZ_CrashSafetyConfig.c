// Settings, saved to $profile:DevZ_CrashSafety\Config.json on first start. Edit there and restart.
class DevZ_CrashSafetyConfig
{
	static const string FOLDER = "$profile:DevZ_CrashSafety\\";
	static const string FILE = FOLDER + "Config.json";
	// Raised when a setting is added whose default doesn't survive loading an older file (lists
	// come back empty), so Upgrade can fill it in
	static const int VERSION = 1;

	// The settings version this file was written for. Don't edit.
	int ConfigVersion = 0;

	int Enabled = 1;

	// Health lost in a heavy crash, as a percent of max health, scaled by crash severity.
	// A crash never takes a player below 1 health.
	float MinHealthLossPercent = 10;
	float MaxHealthLossPercent = 70;

	// Heavy crashes at or above this severity (0 to 1) knock the crew out.
	float KnockoutFromSeverity = 0.3;
	float MinKnockoutSeconds = 15;
	float MaxKnockoutSeconds = 45;

	// Crash damage range mapped to severity 0..1 (vanilla: heavy crash starts at 750, kills at 1200)
	float CarSeverityStart = 750;
	float CarSeverityFull = 1200;
	// Motorbikes use their own scale (vanilla: heavy crash 40, kills at 80)
	float BikeSeverityStart = 40;
	float BikeSeverityFull = 80;

	// Floors, in percent of each part's max health: crashes, falls, fire, engine abuse (over-revving,
	// no coolant, deep water) and decay never take these parts below them, so they end up Badly
	// Damaged instead of Ruined and can still be repaired. Ruined, the hull burns out, and the engine
	// and fuel tank can't be repaired or replaced. Other parts (rotors, wheels, doors...) take full
	// damage. 0 turns a floor off.
	float HullFloorPercent = 10;
	float EngineFloorPercent = 10;
	float FuelTankFloorPercent = 10;
	// 1 = weapons and explosives can't ruin these parts either. 0 = they can (use your PvP/PvE mod
	// to decide where vehicles can be shot at all).
	int FloorsAgainstWeapons = 0;

	// Catch-all damage zones some vehicle models report every collision on (Expansion's Land Rover
	// reports all of them on "Chassis", which passes nothing to the body, wheels or crew). A crash
	// reported on one of these is moved to the body zone nearest the impact instead.
	ref TStringArray CatchAllZones;

	int LogCrashes = 1;
	// Temporary diagnostics (4_World\DevZ_CrashSafety\Debug.c): every hit, level change and
	// destruction of a car or helicopter, with the code path for ruins. Very chatty: leave at 0.
	int DebugLog = 0;

	protected static ref DevZ_CrashSafetyConfig s_Instance;

	void DevZ_CrashSafetyConfig()
	{
		CatchAllZones = DefaultCatchAllZones();
	}

	static TStringArray DefaultCatchAllZones()
	{
		TStringArray zones = new TStringArray();
		zones.Insert("Chassis");
		return zones;
	}

	// Fills in settings added since the file was written
	protected void Upgrade()
	{
		if (ConfigVersion < 1)
			CatchAllZones = DefaultCatchAllZones();
		ConfigVersion = VERSION;
	}

	bool IsCatchAllZone(string zone)
	{
		zone.ToLower();
		foreach (string catchAll : CatchAllZones)
		{
			catchAll.ToLower();
			if (catchAll == zone)
				return true;
		}
		return false;
	}

	static DevZ_CrashSafetyConfig Get()
	{
		if (!s_Instance)
		{
			s_Instance = new DevZ_CrashSafetyConfig();
			if (FileExist(FILE))
			{
				JsonFileLoader<DevZ_CrashSafetyConfig>.JsonLoadFile(FILE, s_Instance);
				s_Instance.Upgrade();
			}
			else
			{
				MakeDirectory(FOLDER);
				s_Instance.ConfigVersion = VERSION;
			}
			// Written back every start, so settings added in an update show up in the file
			JsonFileLoader<DevZ_CrashSafetyConfig>.JsonSaveFile(FILE, s_Instance);
		}
		return s_Instance;
	}

	// Floor for a damage zone ("" = hull), in percent; 0 for parts that aren't held
	float FloorPercent(string zone)
	{
		zone.ToLower();
		switch (zone)
		{
			case "":
			case "globalhealth":
				return HullFloorPercent;
			case "engine":
				return EngineFloorPercent;
			case "fueltank":
				return FuelTankFloorPercent;
		}
		return 0;
	}

	float Severity(float dmg, bool isBike)
	{
		if (isBike)
			return Math.Clamp(Math.InverseLerp(BikeSeverityStart, BikeSeverityFull, dmg), 0, 1);
		return Math.Clamp(Math.InverseLerp(CarSeverityStart, CarSeverityFull, dmg), 0, 1);
	}
}
