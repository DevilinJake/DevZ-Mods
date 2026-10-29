// Some vehicle models report every collision on one catch-all zone (CatchAllZones in the settings):
// Expansion's Land Rover reports all of them on "Chassis", a zone with no wheels or body parts
// linked to it, so crashes never reach the rest of the car or the crew. Such a collision is moved
// to the body zone nearest the impact, and the game's own damage rules take it from there.

// The body zones of one vehicle type: zones with wheels, doors or other zones linked to them, and
// where each sits on the model. Built once per type.
class DevZ_CrashSafetyBodyZones
{
	ref TStringArray m_Names = new TStringArray();
	ref array<vector> m_Positions = new array<vector>();

	void DevZ_CrashSafetyBodyZones(EntityAI vehicle, DevZ_CrashSafetyConfig cfg)
	{
		TStringArray zones = new TStringArray();
		vehicle.GetDamageZones(zones);
		string root = "CfgVehicles " + vehicle.GetType() + " DamageSystem DamageZones ";
		foreach (string zone : zones)
		{
			// Held parts (engine, fuel tank) get their share through the body zones' links
			if (cfg.IsCatchAllZone(zone) || cfg.FloorPercent(zone) > 0)
				continue;

			TStringArray slots = new TStringArray();
			TStringArray links = new TStringArray();
			g_Game.ConfigGetTextArray(root + zone + " inventorySlots", slots);
			g_Game.ConfigGetTextArray(root + zone + " transferToZonesNames", links);
			if (slots.Count() == 0 && links.Count() == 0)
				continue;

			TStringArray points = new TStringArray();
			g_Game.ConfigGetTextArray(root + zone + " memoryPoints", points);
			if (points.Count() == 0 || !vehicle.MemoryPointExists(points[0]))
				continue;

			m_Names.Insert(zone);
			m_Positions.Insert(vehicle.GetMemoryPointPos(points[0]));
		}
	}

	// The body zone nearest a point in model space, or "" if the vehicle has none
	string Nearest(vector localPos)
	{
		string nearest;
		float nearestDistSq = float.MAX;
		foreach (int i, string zone : m_Names)
		{
			float distSq = vector.DistanceSq(localPos, m_Positions[i]);
			if (distSq < nearestDistSq)
			{
				nearestDistSq = distSq;
				nearest = zone;
			}
		}
		return nearest;
	}
}

class DevZ_CrashSafetyContactZones
{
	protected static ref map<string, ref DevZ_CrashSafetyBodyZones> s_BodyZones = new map<string, ref DevZ_CrashSafetyBodyZones>();

	// The zone a collision should count against
	static string Resolve(EntityAI vehicle, string zone, vector localPos)
	{
		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();
		if (!cfg.Enabled || !cfg.IsCatchAllZone(zone))
			return zone;

		DevZ_CrashSafetyBodyZones bodyZones;
		if (!s_BodyZones.Find(vehicle.GetType(), bodyZones))
		{
			bodyZones = new DevZ_CrashSafetyBodyZones(vehicle, cfg);
			s_BodyZones.Insert(vehicle.GetType(), bodyZones);
		}

		string nearest = bodyZones.Nearest(localPos);
		if (nearest == "")
			return zone;
		if (cfg.DebugLog)
			PrintFormat("[DevZ_CrashSafety:debug] %1 contact on '%2' at %3 moved to '%4'", vehicle.GetType(), zone, localPos, nearest);
		return nearest;
	}
}

// Cars, and Expansion's helicopters and boats. Helicopters report their crashes with no zone, so
// they're never moved.
modded class CarScript
{
	override void OnContact(string zoneName, vector localPos, IEntity other, Contact data)
	{
		if (g_Game.IsServer())
			zoneName = DevZ_CrashSafetyContactZones.Resolve(this, zoneName, localPos);
		super.OnContact(zoneName, localPos, other, data);
	}
}
