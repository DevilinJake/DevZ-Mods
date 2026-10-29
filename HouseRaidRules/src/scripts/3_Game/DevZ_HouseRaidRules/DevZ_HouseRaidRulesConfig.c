// Settings, saved to $profile:DevZ_HouseRaidRules\Config.json on first start. Edit there and restart.

// One raid window. Weekday "" means every day. Times are the server's local clock.
class DevZ_HouseRaidWindow
{
	string Weekday;
	int StartHour;
	int StartMinute;
	int DurationMinutes;

	void DevZ_HouseRaidWindow(string weekday = "", int startHour = 0, int startMinute = 0, int durationMinutes = 0)
	{
		Weekday = weekday;
		StartHour = startHour;
		StartMinute = startMinute;
		DurationMinutes = durationMinutes;
	}

	// 0 = Sunday ... 6 = Saturday, -1 = every day
	int WeekdayIndex()
	{
		string name = Weekday;
		name.Trim();
		name.ToLower();
		if (name == "")
			return -1;
		return DevZ_HouseRaidRulesConfig.WEEKDAYS.Find(name);
	}

	bool Contains(int weekdayIndex, int minutesSinceMidnight)
	{
		int day = WeekdayIndex();
		if (day != -1 && day != weekdayIndex)
			return false;
		int start = StartHour * 60 + StartMinute;
		int end = start + DurationMinutes;
		return minutesSinceMidnight >= start && minutesSinceMidnight < end;
	}
}

class DevZ_HouseRaidRulesConfig
{
	static const string FOLDER = "$profile:DevZ_HouseRaidRules\\";
	static const string FILE = FOLDER + "Config.json";
	// Raised when a setting is added whose default doesn't survive loading an older file (lists
	// come back empty), so Upgrade can fill it in
	static const int VERSION = 1;
	static ref TStringArray WEEKDAYS = {"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};

	// The settings version this file was written for. Don't edit.
	int ConfigVersion = 0;

	int Enabled = 1;

	// 1 = the on/off switches come from Ninjins Raid Extension's own MainConfig.json, so there is one
	// place to edit: its Enabled, BlockBulletDamage, BlockExplosiveDamage and BypassForAdmins.
	// 0 = use the four values below instead.
	int FollowNinjinsRaidExtension = 1;
	int BlockBulletDamage = 1;
	int BlockExplosiveDamage = 1;
	// 1 = admins flagged in Ninjins (NinjinsUpgraded_IsAdmin) damage rebuilt parts anywhere, any time
	int BypassForAdmins = 0;
	// Melee hits (axes, hammers...) aren't covered by Ninjins Raid Extension; this one is always ours.
	int BlockMeleeDamage = 1;

	// Ninjins zone types where rebuilt parts can be damaged at all: PvP, Raid, PvE, Safe, Visual,
	// None (no zone and no map-wide rule). Everywhere else the hit is blocked.
	ref TStringArray DamageAllowedInZoneTypes;

	// 1 = inside those zones, hits only land during raid hours
	int RaidHoursOnly = 1;
	// 1 = raid hours are Expansion's (RaidSettings.json "Schedule") when Expansion Base Building is
	// loaded, so the house parts open and close with Expansion's walls. Otherwise Schedule below.
	int UseExpansionRaidSchedule = 1;
	// Our own raid windows, used when Expansion's aren't. Weekday "" = every day; an empty list =
	// always raid hours. Default: Friday 18:00 to Sunday 21:00, server time.
	ref array<ref DevZ_HouseRaidWindow> Schedule;

	// Tell the attacker why the hit did nothing (one notice per player per NoticeWaitSeconds)
	int ShowBlockedNotify = 1;
	float NoticeWaitSeconds = 3;
	string BlockedZoneText = "Rebuilt house parts can only be raided in the PvP zone";
	string BlockedHoursText = "Rebuilt house parts can only be raided during raid hours";

	// Write each blocked hit, and each part kept by the backstop, to the server log ([DevZ_HouseRaidRules] lines)
	int LogBlocked = 1;
	// Debug: also log every hit on a house with built parts (zone, damage type, ammo, source), blocked or not
	int LogAllHits = 0;

	protected static ref DevZ_HouseRaidRulesConfig s_Instance;

	void DevZ_HouseRaidRulesConfig()
	{
		DamageAllowedInZoneTypes = DefaultZoneTypes();
		Schedule = DefaultSchedule();
	}

	static TStringArray DefaultZoneTypes()
	{
		TStringArray types = new TStringArray();
		types.Insert("PvP");
		types.Insert("Raid");
		return types;
	}

	static array<ref DevZ_HouseRaidWindow> DefaultSchedule()
	{
		array<ref DevZ_HouseRaidWindow> windows = new array<ref DevZ_HouseRaidWindow>();
		windows.Insert(new DevZ_HouseRaidWindow("Friday", 18, 0, 360));
		windows.Insert(new DevZ_HouseRaidWindow("Saturday", 0, 0, 1440));
		windows.Insert(new DevZ_HouseRaidWindow("Sunday", 0, 0, 1260));
		return windows;
	}

	// Fills in settings added since the file was written
	protected void Upgrade()
	{
		if (ConfigVersion < 1)
		{
			DamageAllowedInZoneTypes = DefaultZoneTypes();
			Schedule = DefaultSchedule();
		}
		ConfigVersion = VERSION;
	}

	static DevZ_HouseRaidRulesConfig Get()
	{
		if (!s_Instance)
		{
			s_Instance = new DevZ_HouseRaidRulesConfig();
			if (FileExist(FILE))
			{
				JsonFileLoader<DevZ_HouseRaidRulesConfig>.JsonLoadFile(FILE, s_Instance);
				s_Instance.Upgrade();
			}
			else
			{
				MakeDirectory(FOLDER);
				s_Instance.ConfigVersion = VERSION;
			}
			// Written back every start, so settings added in an update show up in the file
			JsonFileLoader<DevZ_HouseRaidRulesConfig>.JsonSaveFile(FILE, s_Instance);
		}
		return s_Instance;
	}

	// ---- The switches, from Ninjins Raid Extension when following it ----

	protected NinjinsRaidExt_Config Ninjins()
	{
		if (!FollowNinjinsRaidExtension)
			return null;
		if (!g_NinjinsRaidExt_Config)
			NinjinsRaidExt_Config.Load();
		return g_NinjinsRaidExt_Config;
	}

	bool RulesOn()
	{
		if (!Enabled)
			return false;
		NinjinsRaidExt_Config ninjins = Ninjins();
		if (ninjins)
			return ninjins.RulesOn();
		return true;
	}

	bool BlocksBullets()
	{
		NinjinsRaidExt_Config ninjins = Ninjins();
		if (ninjins)
			return ninjins.BlockBulletDamage == 1;
		return BlockBulletDamage == 1;
	}

	bool BlocksExplosives()
	{
		NinjinsRaidExt_Config ninjins = Ninjins();
		if (ninjins)
			return ninjins.BlockExplosiveDamage == 1;
		return BlockExplosiveDamage == 1;
	}

	bool BlocksMelee()
	{
		return BlockMeleeDamage == 1;
	}

	bool AdminsBypass()
	{
		NinjinsRaidExt_Config ninjins = Ninjins();
		if (ninjins)
			return ninjins.BypassForAdmins == 1;
		return BypassForAdmins == 1;
	}

	// ---- Zones and hours ----

	bool DamageAllowedIn(string zoneType)
	{
		zoneType.ToLower();
		foreach (string listed : DamageAllowedInZoneTypes)
		{
			listed.Trim();
			listed.ToLower();
			if (listed == zoneType)
				return true;
		}
		return false;
	}

	bool IsRaidTimeNow()
	{
#ifdef EXPANSIONMODBASEBUILDING
		if (UseExpansionRaidSchedule)
		{
			ExpansionRaidSettings raid = GetExpansionSettings().GetRaid(false);
			if (raid && raid.IsLoaded())
				return raid.IsRaidableNow();
		}
#endif
		if (!Schedule || Schedule.Count() == 0)
			return true;
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		int weekday = DayOfWeek(year, month, day);
		int now = hour * 60 + minute;
		foreach (DevZ_HouseRaidWindow window : Schedule)
		{
			if (window && window.Contains(weekday, now))
				return true;
		}
		return false;
	}

	// 0 = Sunday ... 6 = Saturday (Sakamoto's method)
	static int DayOfWeek(int year, int month, int day)
	{
		TIntArray offsets = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
		if (month < 3)
			year = year - 1;
		return (year + year / 4 - year / 100 + year / 400 + offsets[month - 1] + day) % 7;
	}
}
