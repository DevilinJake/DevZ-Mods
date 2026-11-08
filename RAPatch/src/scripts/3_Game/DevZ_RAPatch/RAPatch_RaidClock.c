// Is it raid hours? On its own so the proof doesn't lean on the battle royale's clock; a fork would use DevZBR_Raids.
class RAPatch_RaidClock
{
	static const string DAYS = "SunMonTueWedThuFriSat";
	static const int FOLLOW_SETTINGS = 0;
	static const int FORCED_OPEN = 1;
	static const int FORCED_CLOSED = 2;
	static int Forced = FOLLOW_SETTINGS;	// another mod's test commands can force the clock: the Battle Royale mod's test round does

	// 0 = Sunday. Sakamoto's method.
	static int DayOfWeek(int year, int month, int day)
	{
		int t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
		if (month < 3)
			year -= 1;
		return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
	}

	protected static int Minutes(string hhmm)
	{
		TStringArray parts = new TStringArray;
		hhmm.Split(":", parts);
		if (parts.Count() != 2)
			return -1;
		return parts[0].ToInt() * 60 + parts[1].ToInt();
	}

	// weekDay 0 = Sunday, minute of the day
	static bool IsInWindow(RAPatch_RaidSettings raid, int weekDay, int minute)
	{
		string dayName = DAYS.Substring(weekDay * 3, 3);
		foreach (RAPatch_RaidWindow window : raid.RaidWindows)
		{
			if (window.Day != dayName)
				continue;
			if (minute >= Minutes(window.Start) && minute < Minutes(window.End))
				return true;
		}
		return false;
	}

	static bool IsOpenNow()
	{
		if (Forced == FORCED_OPEN)
			return true;
		if (Forced == FORCED_CLOSED)
			return false;
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		if (!raid.RaidHoursEnabled)
			return true;
		int year, month, day, hour, minute, second;
		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);
		int total = hour * 60 + minute + Math.Round(raid.RaidHoursUtcOffset * 60);
		int weekDay = DayOfWeek(year, month, day);
		if (total >= 1440)
		{
			total -= 1440;
			weekDay = (weekDay + 1) % 7;
		}
		else if (total < 0)
		{
			total += 1440;
			weekDay = (weekDay + 6) % 7;
		}
		return IsInWindow(raid, weekDay, total);
	}
}
