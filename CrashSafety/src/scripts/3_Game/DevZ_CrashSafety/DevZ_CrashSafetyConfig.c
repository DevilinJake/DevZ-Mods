// Settings, saved to $profile:DevZ_CrashSafety\Config.json on first start. Edit there and restart.
class DevZ_CrashSafetyConfig
{
	static const string FOLDER = "$profile:DevZ_CrashSafety\\";
	static const string FILE = FOLDER + "Config.json";

	int Enabled = 1;

	// Health lost in a heavy crash, as a percent of max health, scaled by crash severity.
	// A crash never takes a player below 1 health.
	float MinHealthLossPercent = 10;
	float MaxHealthLossPercent = 70;

	// Heavy crashes at or above this severity (0 to 1) knock the crew out.
	float KnockoutFromSeverity = 0.3;
	float MinKnockoutSeconds = 30;
	float MaxKnockoutSeconds = 60;

	// Crash damage range mapped to severity 0..1 (vanilla: heavy crash starts at 750, kills at 1200)
	float CarSeverityStart = 750;
	float CarSeverityFull = 1200;
	// Motorbikes use their own scale (vanilla: heavy crash 40, kills at 80)
	float BikeSeverityStart = 40;
	float BikeSeverityFull = 80;

	// Crashes, falls and fire never take a vehicle's hull (its overall health) below this percent,
	// so it ends up Badly Damaged instead of Ruined and never burns out. Parts (engine, fuel tank,
	// rotors, wheels, doors...) still take full damage and can be ruined. 0 turns this off.
	float HullFloorPercent = 10;
	// 1 = weapons and explosives can't ruin the hull either. 0 = they can (use your PvP/PvE mod to
	// decide where vehicles can be shot at all).
	int HullFloorAgainstWeapons = 0;

	int LogCrashes = 1;

	protected static ref DevZ_CrashSafetyConfig s_Instance;

	static DevZ_CrashSafetyConfig Get()
	{
		if (!s_Instance)
		{
			s_Instance = new DevZ_CrashSafetyConfig();
			if (FileExist(FILE))
			{
				JsonFileLoader<DevZ_CrashSafetyConfig>.JsonLoadFile(FILE, s_Instance);
			}
			else
			{
				MakeDirectory(FOLDER);
			}
			// Written back every start, so settings added in an update show up in the file
			JsonFileLoader<DevZ_CrashSafetyConfig>.JsonSaveFile(FILE, s_Instance);
		}
		return s_Instance;
	}

	float Severity(float dmg, bool isBike)
	{
		if (isBike)
			return Math.Clamp(Math.InverseLerp(BikeSeverityStart, BikeSeverityFull, dmg), 0, 1);
		return Math.Clamp(Math.InverseLerp(CarSeverityStart, CarSeverityFull, dmg), 0, 1);
	}
}
