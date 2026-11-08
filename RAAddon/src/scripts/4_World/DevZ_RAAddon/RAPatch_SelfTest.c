// "SelfTest": true in Settings.json. Spawns real RA objects at SelfTestX, SelfTestZ, checks they behave as the file says,
// then removes them. Results go to the script log and profiles\DevZ_RAAddon\SelfTest.txt. It reads RA's real functions,
// so a line that passes means RA itself is now using the value. Nothing here needs a player.
class RAPatch_SelfTest
{
	protected static ref RAPatch_SelfTest s_Instance;

	protected int m_Checks;
	protected int m_Failures;
	protected vector m_Base;
	protected TerritoryHQ m_Desk;
	protected BaseBuilding m_Near;
	protected BaseBuilding m_Edge;
	protected BaseBuilding m_Far;
	protected BaseBuilding m_Lonely;
	protected EntityAI m_StoredItem;
	protected CarScript m_Car;
	protected TerritoryHQ m_DoomedDesk;
	protected TerritoryHQ m_LimitDesk;
	protected TerritoryHQ m_BenchDesk;
	protected vector m_DropSpot;
	protected PlayerBase m_Player;
	protected ref array<Object> m_Spawned = new array<Object>;
	protected ref array<BaseBuilding> m_BlastWalls = new array<BaseBuilding>;
	protected ref array<float> m_BlastOffsets = new array<float>;
	protected ref array<TerritoryHQ> m_BlastDesks = new array<TerritoryHQ>;
	protected ref array<float> m_BlastDeskHeights = new array<float>;
	protected TerritoryHQ m_ThreeC4Desk;

	static void Start()
	{
		s_Instance = new RAPatch_SelfTest;
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(s_Instance.Begin, 3000, false);
	}

	protected void Note(string line)
	{
		string text = "[RAPatch test] " + line;
		PrintString(text);
		FileHandle file = OpenFile(RAPatch_Settings.FOLDER + "/SelfTest.txt", FileMode.APPEND);
		if (file != 0)
		{
			FPrintln(file, text);
			CloseFile(file);
		}
	}

	protected void Check(bool good, string what)
	{
		m_Checks++;
		if (good)
		{
			Note("pass " + what);
			return;
		}
		m_Failures++;
		Note("FAIL " + what);
	}

	protected bool Near(float a, float b)
	{
		return Math.AbsFloat(a - b) <= 0.01 + Math.AbsFloat(b) * 0.01;
	}

	protected Object Spawn(string type, vector position)
	{
		position[1] = GetGame().SurfaceY(position[0], position[2]);
		Object spawned = GetGame().CreateObjectEx(type, position, ECE_CREATEPHYSICS | ECE_NOLIFETIME);
		if (spawned)
			m_Spawned.Insert(spawned);
		else
			Note("FAIL could not spawn " + type);
		return spawned;
	}

	protected vector Offset(float x, float z)
	{
		return m_Base + Vector(x, 0, z);
	}

	void Begin()
	{
		RAPatch_Settings settings = RAPatch_Settings.Get();
		m_Base = Vector(settings.SelfTestX, 0, settings.SelfTestZ);
		Note("paper's model: " + GetGame().ConfigGetTextOut("CfgVehicles Paper model") + ", upgrade item's: " + GetGame().ConfigGetTextOut("CfgVehicles RAPatch_DeskUpgrade model"));
		Note("=== start. Decay " + settings.Decay.Enabled + ", radius " + settings.Desk.Radius);

		// A desk left by SelfTestKeepDesk on the last boot: did its modules survive the restart?
		if (settings.SelfTestKeepDesk && TerritoryHQ.All_HQs.Count() > 0)
		{
			CheckLoadedDesk();
			return;
		}

		float radius = settings.Desk.Radius;
		m_Desk = TerritoryHQ.Cast(Spawn("TerritoryHQ", m_Base));
		m_Near = BaseBuilding.Cast(Spawn("BaseBuilding_WoodWall", Offset(10, 0)));
		m_Edge = BaseBuilding.Cast(Spawn("BaseBuilding_FrameWall", Offset(radius - 2, 0)));
		m_Far = BaseBuilding.Cast(Spawn("BaseBuilding_FrameWall", Offset(radius + 10, 0)));
		// RA's own 36 m, to show a bigger or smaller radius really moves the line
		m_Lonely = BaseBuilding.Cast(Spawn("BaseBuilding_MetalWall", Offset(500, 500)));
		m_Player = PlayerBase.Cast(GetGame().CreatePlayer(null, "SurvivorM_Boris", Offset(500, 520), 0, "NONE"));
		if (m_Player)
			m_Spawned.Insert(m_Player);
		else
			Note("FAIL could not make a test character");
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CheckNumbers, 5000, false);
	}

	protected void CheckNumbers()
	{
		RAPatch_Settings settings = RAPatch_Settings.Get();
		float radius = settings.Desk.Radius;

		// The desk
		Check(Near(m_Desk.GetRadius(), radius), "desk radius is " + m_Desk.GetRadius() + ", file says " + radius);
		TerritoryTrigger trigger = m_Desk.GetTerritoryTrigger();
		Check(trigger && Near(trigger.GetCollisionRadius(), radius), "desk trigger radius is " + trigger.GetCollisionRadius() + ", file says " + radius);
		if (settings.Desk.MaxPlayers > 0)
			Check(m_Desk.GetMaxPlayers() == settings.Desk.MaxPlayers, "desk max players is " + m_Desk.GetMaxPlayers());
		else
			Check(m_Desk.GetMaxPlayers() == int.MAX, "desk has no player limit, as RA");
		Check(m_Near.GetCurrentHQ() == m_Desk, "a part 10 m from the desk belongs to it");
		Check(m_Edge.GetCurrentHQ() == m_Desk, "a part 2 m inside the radius (" + (radius - 2) + " m) belongs to it");
		Check(!m_Far.GetCurrentHQ(), "a part 10 m outside the radius (" + (radius + 10) + " m) doesn't");
		Note("desk's upkeep now totals " + m_Desk.GetBaseUpkeepCost() + " nails a day");

		// Upkeep and decay rates
		Check(Near(m_Near.GetCostToUpkeep(), Math.Max(settings.Decay.PartNailsPerDay, RAPatch_Decay.SMALLEST_UPKEEP)), "part upkeep is " + m_Near.GetCostToUpkeep() + " nails a day");
		Check(Near(m_Desk.GetCostToUpkeep(), Math.Max(settings.Decay.DeskNailsPerDay, RAPatch_Decay.SMALLEST_UPKEEP)), "desk upkeep is " + m_Desk.GetCostToUpkeep() + " nails a day");
		float partsCost = m_Desk.GetCostToUpkeep() + m_Near.GetCostToUpkeep() + m_Edge.GetCostToUpkeep();
		Check(Near(m_Desk.GetBaseUpkeepCost(), partsCost), "desk's total matches the desk and its two parts: " + partsCost);
		Check(Near(m_Near.GetTimeToDecay(), RAPatch_Decay.Seconds(settings.Decay.WoodHours)), "wood wall's decay time is " + m_Near.GetTimeToDecay() + " s");
		Check(Near(m_Far.GetTimeToDecay(), RAPatch_Decay.Seconds(settings.Decay.FrameHours)), "frame wall's decay time is " + m_Far.GetTimeToDecay() + " s");
		Check(Near(m_Lonely.GetTimeToDecay(), RAPatch_Decay.Seconds(settings.Decay.MetalHours)), "metal wall's decay time is " + m_Lonely.GetTimeToDecay() + " s");
		Object chest = Spawn("SeaChest", Offset(500, 510));
		ItemBase chestItem = ItemBase.Cast(chest);
		if (chestItem)
		{
			Check(Near(chestItem.GetCostToUpkeep(), Math.Max(settings.Decay.ContainerNailsPerDay, RAPatch_Decay.SMALLEST_UPKEEP)), "sea chest upkeep is " + chestItem.GetCostToUpkeep());
			Check(Near(chestItem.GetTimeToDecay(), RAPatch_Decay.Seconds(settings.Decay.ContainerHours)), "sea chest's decay time is " + chestItem.GetTimeToDecay() + " s");
		}

		// Real decay, through RA's own OnCEIterate: an hour goes by for a part with no desk
		float before = m_Far.GetHealth("GlobalHealth", "Health");
		float maxHealth = m_Far.GetMaxHealth("GlobalHealth", "Health");
		m_Far.OnCEIterate(0, 3600);
		float lost = before - m_Far.GetHealth("GlobalHealth", "Health");
		float expected = 3600 / RAPatch_Decay.Seconds(settings.Decay.FrameHours) * maxHealth;
		Note("an hour with no desk: a frame wall (" + maxHealth + " hp) lost " + lost + ", expected " + expected);
		Check(Near(lost, expected), "decay through RA's OnCEIterate");

		// With a desk but no nails
		before = m_Near.GetHealth("GlobalHealth", "Health");
		maxHealth = m_Near.GetMaxHealth("GlobalHealth", "Health");
		m_Near.OnCEIterate(0, 3600);
		lost = before - m_Near.GetHealth("GlobalHealth", "Health");
		if (settings.Decay.Enabled)
			Check(lost > 0, "a part at a desk with no nails decays: lost " + lost);
		else
			Check(Near(lost, 0), "decay off: a part at a desk with no nails lost " + lost);

		// With nails
		ItemBase nails = ItemBase.Cast(m_Desk.GetInventory().CreateAttachment("Nail"));
		Check(nails != null, "nails go in the desk");
		if (nails)
		{
			nails.SetQuantity(100);
			before = nails.GetQuantity();
			bool paid = m_Desk.ConsumeNails(10);
			float used = before - nails.GetQuantity();
			Note("asked the desk for 10 nails: paid " + paid + ", used " + used);
			if (settings.Decay.Enabled)
				Check(paid && Near(used, 10), "nails are paid when decay is on");
			else
				Check(paid && Near(used, 0), "decay off: no nails are used, and the desk still says paid");
			before = m_Near.GetHealth("GlobalHealth", "Health");
			m_Near.OnCEIterate(0, 3600);
			Check(m_Near.GetHealth("GlobalHealth", "Health") >= before, "a part at a desk with nails doesn't lose health");
		}

		// Build costs
		CheckCost("BaseBuilding_WoodWall", "Wood");
		CheckCost("BaseBuilding_MetalWall", "Metal");
		BaseBuildingItemEntry upgrade = m_Edge.GetUpgradeCost();
		BaseBuildingItemEntry direct = BaseBuilding.GetConstructCost("BaseBuilding_WoodWall");
		Check(upgrade && direct && upgrade.Item == direct.Item && upgrade.Quantity == direct.Quantity, "RA's upgrade cost for a frame wall reads the same value");
		BaseBuildingItemEntry instanceCost = m_Near.GetConstructCost();
		Check(instanceCost && direct && instanceCost.Item == direct.Item && instanceCost.Quantity == direct.Quantity, "RA's cost on the part itself reads the same value");

		// RA's own blueprint action, which refuses a part with no cost. A row with Quantity -1 should make it refuse.
		PlaceHologramActionData placing = new PlaceHologramActionData;
		placing.Type = "BaseBuilding_HologramFloor";
		placing.Position = Offset(0, 80);
		ActionPlaceCurrentBlueprint blueprintAction = new ActionPlaceCurrentBlueprint;
		BaseBuilding placed = blueprintAction.CreateObject(placing);
		bool floorBanned = false;
		foreach (RAPatch_CostRow costRow : settings.Build.Costs)
		{
			if (costRow.Part == "BaseBuilding_FrameFloor" && costRow.Quantity < 0)
				floorBanned = true;
		}
		Check((placed == null) == floorBanned, "RA's blueprint action reads our costs: frame floor banned " + floorBanned + ", placed " + (placed != null));
		if (placed)
			m_Spawned.Insert(placed);

		// Height limit, through RA's own CheckBuildCondition, away from any desk
		float limit = settings.Build.HeightLimitMeters;
		vector low = m_Lonely.GetPosition();
		low[1] = GetGame().SurfaceY(low[0], low[2]) + limit - 1;
		m_Lonely.SetPosition(low);
		Check(BaseBuilding.CheckBuildCondition(m_Player, m_Lonely) == ConstructFailType.NONE, "a part 1 m under the height limit (" + limit + ") is allowed");
		low[1] = low[1] + 2;
		m_Lonely.SetPosition(low);
		Check(BaseBuilding.CheckBuildCondition(m_Player, m_Lonely) == ConstructFailType.HEIGHT, "a part 1 m over the height limit is refused");
		low[1] = GetGame().SurfaceY(low[0], low[2]);
		m_Lonely.SetPosition(low);

		// Placement over something else
		vector mat[4];
		m_Near.GetTransform(mat);
		BaseBuilding overlap = BaseBuilding.Cast(Spawn("BaseBuilding_WoodWall", Offset(0, 60)));
		bool placeable = overlap.EvaluatePlacementCondition(PlacementMode.FREE, mat);
		if (settings.Build.BuildAnywhere)
			Check(placeable, "Build Anywhere: a part can go right on top of another");
		else
			Check(!placeable, "a part can't go right on top of another");

		// The remove window, the way RA's action asks it
		m_Near.OnPlacementComplete(null, m_Near.GetPosition(), m_Near.GetOrientation());
		int age = DateTime.Now() - m_Near.GetPlaceTime();
		bool stillRemovable = (age <= RearmedConstants.ROTATION_COOLDOWN);
		Note("just placed: RA sees age " + age + " s against its fixed " + RearmedConstants.ROTATION_COOLDOWN);
		Check(stillRemovable == (settings.Dismantling.RemoveWindowSeconds > 0), "remove window of " + settings.Dismantling.RemoveWindowSeconds + " s");

		// Desk pack-up rule (mode 1 needs a real player)
		ActionDismantleIntoBox box = new ActionDismantleIntoBox;
		if (settings.Dismantling.DeskPackUp == 0)
			Check(box.RAPatch_MayPackUp(m_Player, m_Desk), "pack-up mode 0: anyone can pack up a desk");
		if (settings.Dismantling.DeskPackUp == 2)
			Check(!box.RAPatch_MayPackUp(m_Player, m_Desk), "pack-up mode 2: nobody can pack up a desk");

		// Tools
		CheckTool("RA_ClawHammer_Blue");
		CheckTool("Hammer");
		CheckTool("Hatchet");
		CheckTool("SledgeHammer");

		CheckModules();

		// No-build zones
		Check(NoBuildZone.All.Count() == settings.Build.NoBuildZones.Count(), "no-build zones: " + NoBuildZone.All.Count());
		foreach (RAPatch_ZoneRow zone : settings.Build.NoBuildZones)
			Check(!NoBuildZone.HasPermissionsAtPosition(Vector(zone.X, GetGame().SurfaceY(zone.X, zone.Z), zone.Z)), "the middle of a no-build zone refuses building");

		// Raid clock
		RAPatch_RaidSettings windows = new RAPatch_RaidSettings;
		windows.RaidWindows.Insert(new RAPatch_RaidWindow("Fri", "18:00", "23:30"));
		Check(RAPatch_RaidClock.DayOfWeek(2026, 11, 7) == 6, "7 Nov 2026 is a Saturday");
		Check(RAPatch_RaidClock.IsInWindow(windows, 5, 18 * 60), "Fri 18:00 is in the window");
		Check(!RAPatch_RaidClock.IsInWindow(windows, 5, 18 * 60 - 1), "Fri 17:59 isn't");
		Check(!RAPatch_RaidClock.IsInWindow(windows, 6, 19 * 60), "Sat 19:00 isn't");
		RA_C4 closedTest = RA_C4.Cast(Spawn("RA_C4", Offset(0, 300)));
		closedTest.SetArmState(true);
		Check(closedTest.IsArmed() == RAPatch_RaidClock.IsOpenNow(), "a charge arms only in raid hours. Open now: " + RAPatch_RaidClock.IsOpenNow() + ", armed: " + closedTest.IsArmed());
		closedTest.SetArmState(false);

		CheckRespawn();
		CheckVehicleLock();
		CheckRepair();
		StartBuildingLimits();
		CheckBench();
		StartBlast();
		CheckRecipes();
	}

	protected void CheckLoadedDesk()
	{
		TerritoryHQ desk = TerritoryHQ.All_HQs[0];
		// The bench desk is kept too, and it has only its own module
		foreach (TerritoryHQ candidate : TerritoryHQ.All_HQs)
		{
			if (candidate && candidate.RAPatch_BenchQueueCount() == 0)
				desk = candidate;
		}
		float want = RAPatch_Settings.Get().Desk.Radius;
		foreach (RAPatch_ModuleRow row : RAPatch_Settings.Get().Desk.Modules)
		{
			Check(desk.RAPatch_HasModule(row.Name), "after a restart, the desk still has " + row.Name);
			want += row.RadiusAdded;
		}
		Check(Near(desk.GetRadius(), want), "after a restart, the radius is " + desk.GetRadius() + ", expected " + want);
		if (RAPatch_Settings.Get().Desk.Modules.Count() > 0)
			Check(desk.RAPatch_WebhookIsSet(), "after a restart, the webhook is still saved");
		array<Object> nearby = new array<Object>;
		vector carSpot = Offset(-60, 0);
		carSpot[1] = GetGame().SurfaceY(carSpot[0], carSpot[2]);
		GetGame().GetObjectsAtPosition3D(carSpot, 40, nearby, null);
		bool foundCar = false;
		foreach (Object thing : nearby)
		{
			CarScript car = CarScript.Cast(thing);
			if (car)
			{
				foundCar = true;
				Check(car.RAPatch_IsLocked(), "after a restart, the car's still locked");
				Check(car.RAPatch_LockUserAllowed("selftest-owner"), "and its owner's still allowed");
				Check(car.RAPatch_IdIsDenied("selftest-other"), "and a stranger's still shut out");
				Check(car.RAPatch_TryCode("selftest-other", "1234") == "", "and its code still works");
			}
		}
		Check(foundCar, "the test car came back");
		foreach (TerritoryHQ loaded : TerritoryHQ.All_HQs)
		{
			if (loaded && loaded.RAPatch_BenchQueueCount() > 0)
			{
				Check(loaded.RAPatch_BenchResultCount() == 1, "after a restart, the workbench's finished item is still there");
				Check(loaded.RAPatch_BenchBuildStatus().L > 0, "and so is the job it was making");
			}
		}
		TerritoryTrigger trigger = desk.GetTerritoryTrigger();
		Check(trigger && Near(trigger.GetCollisionRadius(), want), "and the trigger too");
		Note("=== done: " + m_Checks + " checks, " + m_Failures + " failed");
	}

	protected void CheckModules()
	{
		RAPatch_DeskSettings rules = RAPatch_Settings.Get().Desk;
		if (rules.Modules.Count() == 0)
		{
			Note("no modules in the settings: not tested");
			return;
		}
		ItemBase wrong = ItemBase.Cast(Spawn("Apple", Offset(3, 3)));
		string refusal = m_Desk.RAPatch_WhyNotInstall(wrong);
		Check(refusal != "", "an apple isn't a module: " + refusal);
		float radius = rules.Radius;
		int players = rules.MaxPlayers;
		float multiplier = 1;
		bool storage = false;
		foreach (RAPatch_ModuleRow row : rules.Modules)
		{
			ItemBase full = ItemBase.Cast(Spawn(row.Items[0], Offset(3, 3)));
			if (full && full.GetInventory().CreateInInventory("Apple"))
				Check(m_Desk.RAPatch_WhyNotInstall(full) != "", "a " + row.Items[0] + " with something in it is refused");
			ItemBase item = ItemBase.Cast(Spawn(row.Items[0], Offset(3, 3)));
			Check(m_Desk.RAPatch_WhyNotInstall(item) == "", row.Items[0] + " can be installed as " + row.Name);
			Check(m_Desk.RAPatch_Install(item), row.Name + " installed");
			Check(m_Desk.RAPatch_HasModule(row.Name), "the desk says it has " + row.Name);
			ItemBase again = ItemBase.Cast(Spawn(row.Items[0], Offset(3, 3)));
			Check(m_Desk.RAPatch_WhyNotInstall(again) != "", "a second " + row.Name + " is refused");
			radius += row.RadiusAdded;
			players += row.PlayersAdded;
			multiplier *= row.UpkeepMultiplier;
			if (row.Storage)
				storage = true;
		}
		Check(Near(m_Desk.GetRadius(), radius), "with its modules, the radius is " + m_Desk.GetRadius());
		Check(Near(m_Desk.GetTerritoryTrigger().GetCollisionRadius(), radius), "and the trigger too");
		if (rules.MaxPlayers > 0)
			Check(m_Desk.GetMaxPlayers() == players, "and max players is " + m_Desk.GetMaxPlayers());
		ItemBase nails = ItemBase.Cast(m_Desk.FindAttachmentBySlotName("HQ_Nails"));
		if (nails && RAPatch_Settings.Get().Decay.Enabled)
		{
			float before = nails.GetQuantity();
			m_Desk.ConsumeNails(20);
			float used = before - nails.GetQuantity();
			Note("asked for 20 nails with an upkeep multiplier of " + multiplier + ": used " + used);
			Check(Math.AbsFloat(used - 20 * multiplier) <= 1, "the upkeep module changes what's paid");
		}
		Check(m_Desk.CanDisplayCargo() == storage, "desk storage is open: " + storage);
		CheckAlarm();
		CheckClientCopy();
		if (storage)
		{
			m_StoredItem = m_Desk.GetInventory().CreateInInventory("Apple");
			Check(m_StoredItem != null, "something can go in the desk's storage");
		}
		TerritoryHQ other = TerritoryHQ.Cast(Spawn("TerritoryHQ", Offset(radius + 5, 0)));
		if (other)
		{
			foreach (RAPatch_ModuleRow radiusRow : rules.Modules)
			{
				if (radiusRow.RadiusAdded > 0)
				{
					RAPatch_CheckOverlap(other, radiusRow);
				}
			}
			GetGame().ObjectDelete(other);
		}
	}

	protected void CheckAlarm()
	{
		bool hasRow = false;
		foreach (RAPatch_ModuleRow row : RAPatch_Settings.Get().Desk.Modules)
		{
			if (row.RaidAlarm)
				hasRow = true;
		}
		Check(m_Desk.RAPatch_HasRaidAlarm() == hasRow, "the desk's raid alarm is on: " + hasRow);
		Check(!RAPatch_Webhook.IsValid("https://example.com/api/webhooks/1/abc"), "a webhook on another site is refused");
		Check(!RAPatch_Webhook.IsValid("http://discord.com/api/webhooks/1/abc"), "a webhook without https is refused");
		Check(!RAPatch_Webhook.IsValid("https://discord.com/api/webhooks/"), "an empty webhook is refused");
		Check(RAPatch_Webhook.IsValid("https://discord.com/api/webhooks/123456789012345678/abcdefghijklmnopqrstuvwxyzABCDEF_-0123"), "a Discord webhook is accepted");
		Check(!RAPatch_Webhook.IsValid("https://discord.com/api/webhooks/1/abc"), "one that's cut short is refused");
		Check(!RAPatch_Webhook.IsValid("https://discord.com/api/webhooks/wrongaddress"), "so is a made-up ending");
		if (!hasRow)
			return;

		// Nothing goes to Discord from here: the post is only counted
		RAPatch_Webhook.DryRun = true;
		RAPatch_Webhook.DryRunCount = 0;
		m_Desk.RAPatch_RaidHit("Gunfire", "Test wall", null);
		Check(RAPatch_Webhook.DryRunCount == 0, "no webhook saved: no post");
		m_Desk.RAPatch_SetWebhook("https://discord.com/api/webhooks/123456789012345678/abcdefghijklmnopqrstuvwxyzABCDEF_-0123");
		m_Desk.RAPatch_RaidHit("Gunfire", "Test wall", null);
		Check(RAPatch_Webhook.DryRunCount == 1, "a hit posts an alert: " + RAPatch_Webhook.DryRunLast);
		m_Desk.RAPatch_RaidHit("Gunfire", "Test wall", null);
		m_Desk.RAPatch_RaidHit("Melee", "Test door", null);
		Check(RAPatch_Webhook.DryRunCount == 1, "more hits during the cooldown don't post");

		// A real hit, through the game's own damage route
		m_Near.RAPatch_ReportHitForTest();
		Check(RAPatch_Webhook.DryRunCount == 1, "a zombie's or animal's hit is ignored");

		RAPatch_DeskStatus status = m_Desk.RAPatch_BuildStatus();
		string statusJson;
		JsonSerializer serializer = new JsonSerializer;
		serializer.WriteToString(status, false, statusJson);
		Check(statusJson.Length() < 400, "the dashboard's message is short enough: " + statusJson.Length());
		Check(statusJson.IndexOf("discord.com") == -1, "the dashboard's message doesn't contain the webhook");
		Check(status.WebhookSet, "and it says one is saved");
		Check(status.Parts >= 2, "it counts the desk's parts: " + status.Parts);
		Note("dashboard status: " + statusJson);
		if (!RAPatch_Settings.Get().SelfTestKeepDesk)
			m_Desk.RAPatch_SetWebhook("");

		// Killed: a last message, and the webhook goes with it
		m_DoomedDesk = TerritoryHQ.Cast(Spawn("TerritoryHQ", Offset(-100, -100)));
		foreach (RAPatch_ModuleRow alarmRow : RAPatch_Settings.Get().Desk.Modules)
		{
			if (alarmRow.RaidAlarm)
				m_DoomedDesk.RAPatch_Install(ItemBase.Cast(Spawn(alarmRow.Items[0], Offset(-100, -95))));
		}
		m_DoomedDesk.RAPatch_SetWebhook("https://discord.com/api/webhooks/123456789012345678/abcdefghijklmnopqrstuvwxyzABCDEF_-0123");
		RAPatch_Webhook.DryRunLast = "";
		m_DoomedDesk.SetHealth("", "", 0);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CheckDoomedDesk, 1000, false);
	}

	protected void CheckDoomedDesk()
	{
		Check(m_DoomedDesk && !m_DoomedDesk.RAPatch_WebhookIsSet(), "a killed desk forgets its webhook");
		Check(RAPatch_Webhook.DryRunLast.Contains("destroyed"), "after posting that it's destroyed: " + RAPatch_Webhook.DryRunLast);
	}

	protected void CheckClientCopy()
	{
		RAPatch_Settings settings = RAPatch_Settings.Get();
		string text = settings.ToJsonForClients();
		RAPatch_Settings sent = RAPatch_Settings.FromJson(text);
		Check(sent != null, "the copy for clients can be read back");
		if (!sent)
			return;
		RAPatch_Settings defaults = new RAPatch_Settings;
		Check(sent.Raiding.C4Damage == defaults.Raiding.C4Damage, "clients don't get this server's C4 damage");
		Check(sent.Raiding.C4FuseSeconds == defaults.Raiding.C4FuseSeconds, "or its fuse");
		Check(sent.Raiding.RaidWindows.Count() == 0, "or its raid hours");
		Check(sent.Decay.PartNailsPerDay == defaults.Decay.PartNailsPerDay, "or its decay numbers");
		Check(sent.Build.NoBuildZones.Count() == 0, "or its no-build list");
		Check(!sent.SelfTest, "or test settings");
		Check(sent.Crafting.Recipes.Count() == settings.Crafting.Recipes.Count(), "they do get the recipes");
		Check(sent.Desk.Modules.Count() == settings.Desk.Modules.Count(), "and the modules");
		Note("copy for clients: " + text.Length() + " characters");
	}

	// A fresh desk with a radius module, beside another desk
	protected void RAPatch_CheckOverlap(TerritoryHQ other, RAPatch_ModuleRow row)
	{
		ItemBase item = ItemBase.Cast(Spawn(row.Items[0], Offset(3, 3)));
		string why = other.RAPatch_WhyNotInstall(item);
		bool mayOverlap = RAPatch_Settings.Get().Desk.RadiusMayOverlap;
		Check((why != "") != mayOverlap, "a radius module that would overlap another desk: refusal '" + why + "', allowed by the file: " + mayOverlap);
	}

	protected void CheckRemoval()
	{
		if (m_StoredItem)
		{
			if (m_Desk.RAPatch_LastIsStorage())
				Check(!m_Desk.RAPatch_RemoveLast(null), "storage with something in it can't be taken out");
			GetGame().ObjectDelete(m_StoredItem);
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(FinishRemoval, 1000, false);
	}

	protected void FinishRemoval()
	{
		RAPatch_DeskSettings rules = RAPatch_Settings.Get().Desk;
		while (m_Desk.RAPatch_ModuleCount() > 0)
		{
			if (!m_Desk.RAPatch_RemoveLast(null))
				break;
		}
		Check(m_Desk.RAPatch_ModuleCount() == 0, "all modules taken back out");
		Check(Near(m_Desk.GetRadius(), rules.Radius), "the radius is back to " + m_Desk.GetRadius());
		array<Object> nearby = new array<Object>;
		GetGame().GetObjectsAtPosition(m_Desk.GetPosition(), 3, nearby, null);
		int returned = 0;
		foreach (Object thing : nearby)
		{
			foreach (RAPatch_ModuleRow row : rules.Modules)
			{
				if (thing.GetType() == row.Items[0])
				{
					returned++;
					m_Spawned.Insert(thing);
				}
			}
		}
		Check(returned >= rules.Modules.Count(), "the items came back: " + returned);
		FinishCleanup();
	}

	protected void CheckRespawn()
	{
		RAPatch_RespawnSettings rules = RAPatch_Settings.Get().Respawn;
		RAPatch_Respawn respawn = RAPatch_Respawn.Get();
		string id = "selftest-player";
		respawn.Forget(id);
		vector spot;
		string kind;
		string note;

		// Who can pick up a bag
		Check(RAPatch_BagRules.MayPickUp(0, false, false), "pickup 0: anyone");
		Check(RAPatch_BagRules.MayPickUp(1, true, false), "pickup 1: the deployer can");
		Check(!RAPatch_BagRules.MayPickUp(1, false, true), "pickup 1: a teammate can't");
		Check(RAPatch_BagRules.MayPickUp(2, false, true), "pickup 2: a teammate can, with the deployer logged off");
		Check(RAPatch_BagRules.MayPickUp(2, true, false), "pickup 2: the deployer can");
		Check(!RAPatch_BagRules.MayPickUp(2, false, false), "pickup 2: a stranger can't");

		// Nothing set: vanilla
		Check(!respawn.Decide(id, spot, kind, note), "no bag and no desk: vanilla's spawn points");

		// The desk
		m_Desk.AuthorizePlayer(id);
		kind = "";
		bool wasDeskWhenNoBag = rules.DeskWhenNoBag;
		rules.DeskWhenNoBag = false;
		Check(!respawn.Decide(id, spot, kind, note), "bags on, none set, authorised on a desk: vanilla's, the bag is the only way back");
		rules.DeskWhenNoBag = true;
		kind = "";
		Check(respawn.Decide(id, spot, kind, note) && kind == RAPatch_Respawn.HQ, "with DeskWhenNoBag on: the desk stands in");
		rules.DeskWhenNoBag = wasDeskWhenNoBag;
		RAPatch_RespawnSettings freshRules = new RAPatch_RespawnSettings;
		Check(!freshRules.DeskWhenNoBag, "DeskWhenNoBag is off by default");
		bool wasBagsOn = rules.BagsEnabled;
		rules.BagsEnabled = false;
		kind = "";
		Check(respawn.Decide(id, spot, kind, note) && kind == RAPatch_Respawn.HQ, "bags off, authorised on a desk: respawn there");
		Check(vector.Distance(spot, m_Desk.GetPosition()) < 4, "beside the desk: " + vector.Distance(spot, m_Desk.GetPosition()) + " m");
		bool wasHq = rules.HQEnabled;
		rules.HQEnabled = false;
		Check(!respawn.Decide(id, spot, kind, note), "both off: vanilla's spawn points");
		rules.HQEnabled = wasHq;
		rules.BagsEnabled = wasBagsOn;

		// Bags on: the bag
		RAPatch_SleepingBagDeployed_Base bag = RAPatch_SleepingBagDeployed_Base.Cast(Spawn("RAPatch_SleepingBag_Green_Deployed", Offset(-20, -20)));
		Check(bag != null, "a laid-out bag spawns");
		Note("bag's model: " + GetGame().ConfigGetTextOut("CfgVehicles RAPatch_SleepingBag_Green_Deployed model") + ", max health " + bag.GetMaxHealth("", ""));
		respawn.SetBag(id, bag);
		kind = "";
		Check(respawn.Decide(id, spot, kind, note) && kind == RAPatch_Respawn.BAG, "a bag and a desk: the bag");
		Check(vector.Distance(spot, bag.GetPosition()) < 1, "at the bag");
		bool wasBags = rules.BagsEnabled;
		rules.BagsEnabled = false;
		kind = "";
		Check(respawn.Decide(id, spot, kind, note) && kind == RAPatch_Respawn.HQ, "bags off: the desk");
		rules.BagsEnabled = wasBags;

		// One bag at a time
		RAPatch_SleepingBagDeployed_Base second = RAPatch_SleepingBagDeployed_Base.Cast(Spawn("RAPatch_SleepingBag_Red_Deployed", Offset(-25, -20)));
		respawn.SetBag(id, second);
		Check(respawn.FindBag(id) == second, "setting a second bag replaces the first");

		// Any class can be a bag: an object from "another mod", here a sea chest
		ItemBase chest2 = ItemBase.Cast(Spawn("SeaChest", Offset(-15, -20)));
		Check(!RAPatch_Respawn.IsBag(chest2), "a sea chest isn't a bag by default");
		rules.BagClasses.Insert("SeaChest");
		Check(RAPatch_Respawn.IsBag(chest2), "listed in Respawn.BagClasses, it is");
		respawn.SetBag(id, chest2);
		kind = "";
		Check(respawn.Decide(id, spot, kind, note) && kind == RAPatch_Respawn.BAG, "and a player can respawn at it");
		rules.BagClasses.RemoveItem("SeaChest");
		Check(respawn.FindBag(id) == null, "taken off the list, it no longer counts");

		// Moved away: no longer their bag
		respawn.SetBag(id, second);
		second.SetPosition(second.GetPosition() + "20 0 0");
		Check(respawn.FindBag(id) == null, "a bag that's been moved isn't their bag any more");
		respawn.SetBag(id, bag);
		RAPatch_Respawn reloaded = new RAPatch_Respawn;
		Check(reloaded.FindBag(id) == bag, "a set bag is read back from the file, as after a restart");

		// Cooldowns, wear, and passing to the next
		float before = bag.GetHealth("", "");
		respawn.RecordUse(id, RAPatch_Respawn.BAG);
		float lost = before - bag.GetHealth("", "");
		Check(Near(lost, bag.GetMaxHealth("", "") * rules.BagWearPercent / 100), "a respawn wears the bag: " + lost);
		Check(Near(respawn.SecondsLeft(id, RAPatch_Respawn.BAG), rules.BagCooldownSeconds), "the bag's first cooldown is the base: " + respawn.SecondsLeft(id, RAPatch_Respawn.BAG) + " s");
		kind = "";
		note = "";
		Check(!respawn.Decide(id, spot, kind, note) && note != "", "bag on cooldown: wait for it, the desk isn't a way round. Note: " + note);
		int planLeft;
		string planNote;
		Check(respawn.Plan(id, spot, kind, planLeft, planNote) && kind == RAPatch_Respawn.BAG && planLeft > 0, "the plan says the bag, in " + planLeft + " s");
		wasBagsOn = rules.BagsEnabled;
		rules.BagsEnabled = false;
		respawn.RecordUse(id, RAPatch_Respawn.HQ);
		note = "";
		Check(!respawn.Decide(id, spot, kind, note) && note != "", "bags off, desk on cooldown: vanilla's. Note: " + note);
		rules.BagsEnabled = wasBagsOn;
		m_Desk.DeauthorizePlayer(id);

		// The cooldown penalty. Pretend each cooldown ended a little while ago, inside the window
		float wasMax = rules.CooldownMaxSeconds;
		rules.CooldownMaxSeconds = 0;
		int window = rules.CooldownPenaltyWindowSeconds;
		int base = rules.BagCooldownSeconds;
		int extra = rules.CooldownPenaltySeconds;
		respawn.EndCooldownForTest(id, window - 5);
		respawn.RecordUse(id, RAPatch_Respawn.BAG);
		Check(Near(respawn.SecondsLeft(id, RAPatch_Respawn.BAG), base + extra), "inside the window, the next cooldown grows: " + respawn.SecondsLeft(id, RAPatch_Respawn.BAG) + " s");
		respawn.EndCooldownForTest(id, 1);
		respawn.RecordUse(id, RAPatch_Respawn.BAG);
		Check(Near(respawn.SecondsLeft(id, RAPatch_Respawn.BAG), base + extra * 2), "and again it adds up: " + respawn.SecondsLeft(id, RAPatch_Respawn.BAG) + " s");
		respawn.EndCooldownForTest(id, 1);
		rules.CooldownMaxSeconds = base + extra;
		respawn.RecordUse(id, RAPatch_Respawn.BAG);
		Check(Near(respawn.SecondsLeft(id, RAPatch_Respawn.BAG), base + extra), "it stops at the limit: " + respawn.SecondsLeft(id, RAPatch_Respawn.BAG) + " s");
		rules.CooldownMaxSeconds = wasMax;
		respawn.EndCooldownForTest(id, window + 5);
		respawn.RecordUse(id, RAPatch_Respawn.BAG);
		Check(Near(respawn.SecondsLeft(id, RAPatch_Respawn.BAG), base), "outside the window, it's back to the base: " + respawn.SecondsLeft(id, RAPatch_Respawn.BAG) + " s");

		// What a player comes back with, on a real character
		m_Player.SetHealth01("GlobalHealth", "Health", 1);
		m_Player.SetHealth01("GlobalHealth", "Blood", 1);
		respawn.ApplyRespawnState(m_Player);
		Check(Near(m_Player.GetHealth01("GlobalHealth", "Health"), 1), "health stays full");
		Check(Near(m_Player.GetHealth01("GlobalHealth", "Blood"), 1), "blood stays full");
		Check(Near(m_Player.GetStatWater().Get() / m_Player.GetStatWater().GetMax() * 100, rules.RespawnWaterPercent), "water is the file's: " + m_Player.GetStatWater().Get());
		Check(Near(m_Player.GetStatEnergy().Get() / m_Player.GetStatEnergy().GetMax() * 100, rules.RespawnEnergyPercent), "energy is the file's: " + m_Player.GetStatEnergy().Get());
		respawn.Forget(id);

		// Deploying in someone's territory
		int rule = RAPatch_Settings.Get().Desk.DeployRule;
		ItemBase trap = ItemBase.Cast(Spawn("LandMineTrap", Offset(-30, -20)));
		Check(trap && trap.CanPlaceInEnemyTerritory() == (rule != 0), "deploy rule " + rule + ": a trap, " + trap.CanPlaceInEnemyTerritory());
		ItemBase packed = ItemBase.Cast(Spawn("RAPatch_SleepingBag_Green", Offset(-40, -20)));
		Check(packed && packed.CanPlaceInEnemyTerritory() == (rule == 1), "deploy rule " + rule + ": a bag, " + packed.CanPlaceInEnemyTerritory());
		ItemBase charge = ItemBase.Cast(Spawn("RA_C4", Offset(-35, -20)));
		Check(charge && charge.CanPlaceInEnemyTerritory(), "charges can always be placed");
		Check(!m_Near.CanPlaceInEnemyTerritory(), "and building parts never can");

		CheckBagTerritoryRule();
	}

	// With the rule on, a set bag only works while it's in the player's territory
	protected void CheckBagStillInTerritory(string owner, vector inside)
	{
		RAPatch_Respawn respawn = RAPatch_Respawn.Get();
		respawn.Forget(owner);
		Object bag = Spawn("RAPatch_SleepingBag_Green_Deployed", inside);
		if (!bag)
			return;
		respawn.SetBag(owner, bag);
		vector spot;
		string kind;
		string note;
		Check(respawn.Decide(owner, spot, kind, note) && kind == RAPatch_Respawn.BAG, "bag rule on: a bag in your territory works");
		m_Desk.DeauthorizePlayer(owner);
		note = "";
		Check(!respawn.Decide(owner, spot, kind, note) && note != "", "taken off the desk: the bag no longer works, and there's no desk either. " + note);
		m_Desk.AuthorizePlayer(owner);
		respawn.Forget(owner);
	}

	// Respawn.BagsNeedOwnTerritory
	protected void CheckBagTerritoryRule()
	{
		RAPatch_RespawnSettings rules = RAPatch_Settings.Get().Respawn;
		bool wasOn = rules.BagsNeedOwnTerritory;
		string owner = "selftest-bag-owner";
		string stranger = "selftest-bag-stranger";
		vector inside = Offset(10, 10);
		inside[1] = GetGame().SurfaceY(inside[0], inside[2]);
		vector outside = Offset(RAPatch_Settings.Get().Desk.Radius + 100, 0);
		outside[1] = GetGame().SurfaceY(outside[0], outside[2]);
		m_Desk.AuthorizePlayer(owner);

		// The rule's own answers
		Check(RAPatch_BagRules.SpotRefusal(true, true) == "", "bag rule: inside your own territory is fine");
		Check(RAPatch_BagRules.SpotRefusal(false, false) != "", "bag rule: outside any territory is refused");
		Check(RAPatch_BagRules.SpotRefusal(true, false) != "", "bag rule: someone else's territory is refused");

		// Off: anywhere, for anyone, as before this setting existed
		rules.BagsNeedOwnTerritory = false;
		Check(RAPatch_Respawn.BagSpotRefusal(null, outside) == "", "bag rule off: a bag can go outside any territory");
		Check(RAPatch_Respawn.BagSpotRefusal(null, inside) == "", "bag rule off: the rule doesn't ask about territory");
		RAPatch_RespawnSettings defaults = new RAPatch_RespawnSettings;
		Check(defaults.BagsNeedOwnTerritory, "the bag rule is on by default");
		Check(defaults.BagCooldownSeconds == 180 && defaults.HQCooldownSeconds == 180 && defaults.CooldownPenaltySeconds == 120 && defaults.CooldownPenaltyWindowSeconds == 120, "cooldown defaults: 3 min, plus 2 min within 2 min of the last ending");

		// On, against a real desk
		rules.BagsNeedOwnTerritory = true;
		Check(RAPatch_Respawn.BagSpotRefusalFor(owner, inside) == "", "bag rule on: authorised, inside the desk's radius: allowed");
		CheckBagStillInTerritory(owner, inside);
		Check(RAPatch_Respawn.BagSpotRefusalFor(stranger, inside) != "", "bag rule on: not authorised, inside the radius: refused");
		Check(RAPatch_Respawn.BagSpotRefusalFor(owner, outside) != "", "bag rule on: authorised, but outside the radius: refused");
		Check(RAPatch_Respawn.BagSpotRefusal(m_Player, inside) != "", "bag rule on: a character with no identity is nobody's teammate");

		// The server's last check, through the bag's own placing code. The packed bag stays, and no laid-out one appears.
		RAPatch_SleepingBag_Base packed = RAPatch_SleepingBag_Base.Cast(Spawn("RAPatch_SleepingBag_Red", outside));
		Check(packed != null, "a packed bag spawns");
		if (packed)
		{
			packed.OnPlacementComplete(m_Player, outside, "0 0 0");
			array<Object> nearby = new array<Object>;
			GetGame().GetObjectsAtPosition3D(outside, 3, nearby, null);
			bool laidOut = false;
			foreach (Object thing : nearby)
			{
				if (RAPatch_SleepingBagDeployed_Base.Cast(thing))
					laidOut = true;
			}
			Check(!laidOut, "bag rule on: refused at the placed spot, no laid-out bag appeared there");
			Check(!packed.IsSetForDeletion(), "bag rule on: and it stays packed");

			rules.BagsNeedOwnTerritory = false;
			packed.OnPlacementComplete(m_Player, outside, "0 0 0");
			nearby.Clear();
			GetGame().GetObjectsAtPosition3D(outside, 3, nearby, null);
			foreach (Object placed : nearby)
			{
				if (RAPatch_SleepingBagDeployed_Base.Cast(placed))
				{
					laidOut = true;
					m_Spawned.Insert(placed);
				}
			}
			Check(laidOut, "bag rule off: the same bag lays out at that spot");
		}

		m_Desk.DeauthorizePlayer(owner);
		rules.BagsNeedOwnTerritory = wasOn;
	}

	protected void CheckVehicleLock()
	{
		RAPatch_VehicleLockSettings rules = RAPatch_VehicleLocks.Rules();
		string owner = "selftest-owner";
		string stranger = "selftest-stranger";
		Check(RAPatch_VehicleLocks.IsValidCode("1234"), "1234 is a valid code");
		Check(!RAPatch_VehicleLocks.IsValidCode("123"), "123 isn't");
		Check(!RAPatch_VehicleLocks.IsValidCode("12a4"), "12a4 isn't");

		m_Car = CarScript.Cast(Spawn("OffroadHatchback", Offset(-60, 0)));
		Check(m_Car != null, "a car spawns");
		if (!m_Car)
			return;
		Check(m_Car.RAPatch_LockState() == RAPatch_VehicleLocks.NONE, "a new car has no lock");
		Check(!m_Car.RAPatch_IsLocked(), "and isn't locked");
		Check(!m_Car.RAPatch_IdIsDenied(stranger), "and shuts nobody out");

		// Fitting uses up the item
		ItemBase lock = ItemBase.Cast(Spawn(rules.LockItem, Offset(-60, 5)));
		Check(m_Car.RAPatch_FitLock(lock), "a " + rules.LockItem + " is fitted");
		Check(m_Car.RAPatch_LockState() == RAPatch_VehicleLocks.PENDING, "waiting for a code");
		Check(!m_Car.RAPatch_IdIsDenied(stranger), "with no code yet, nobody's shut out");
		ItemBase spare = ItemBase.Cast(Spawn(rules.LockItem, Offset(-60, 6)));
		Check(!m_Car.RAPatch_FitLock(spare), "a second lock is refused");

		// Code
		Check(!m_Car.RAPatch_SetCode(owner, "12"), "a short code is refused");
		Check(m_Car.RAPatch_SetCode(owner, "1234"), "a code is set");
		Check(m_Car.RAPatch_IsLocked(), "and the car's locked");
		Check(!m_Car.RAPatch_IdIsDenied(owner), "the one who set it can use it, with nothing to unlock");
		Check(m_Car.RAPatch_IdIsDenied(stranger), "a stranger's shut out");

		// Wrong code, then a wait, then the right one
		string wrong = m_Car.RAPatch_TryCode(stranger, "9999");
		Check(wrong != "", "a wrong code is refused: " + wrong);
		Check(m_Car.RAPatch_IdIsDenied(stranger), "and they're still shut out");
		string tooSoon = m_Car.RAPatch_TryCode(stranger, "1234");
		if (rules.WrongCodeWaitSeconds > 0)
			Check(tooSoon != "", "straight after, even the right code has to wait: " + tooSoon);
		Check(m_Car.RAPatch_TryCode("selftest-friend", "1234") == "", "someone else with the right code gets in");
		Check(!m_Car.RAPatch_IdIsDenied("selftest-friend"), "and stays allowed");
		Check(m_Car.RAPatch_IdIsDenied(stranger), "without letting anyone else in");

		// The alarm starts and stops
		if (rules.AlarmOnBreak)
		{
			m_Car.RAPatch_SoundAlarm(true);
			Check(m_Car.RAPatch_AlarmIsRunning(), "forcing it starts the alarm");
			m_Car.RAPatch_SoundAlarm(false);
			Check(!m_Car.RAPatch_AlarmIsRunning(), "stopping stops it");
		}

		// Forcing it
		Check(!m_Car.RAPatch_BreakLock(0.99), "a bad roll: it holds");
		Check(m_Car.RAPatch_IsLocked(), "still locked");

		// The admin's test: as if they'd never entered it
		m_Car.RAPatch_ForgetUser(owner);
		Check(m_Car.RAPatch_IdIsDenied(owner), "an admin who forgets the code is shut out");
		Check(m_Car.RAPatch_TryCode(owner, "1234") == "", "until they enter it again");

		// Removed: anyone can use it again
		Check(m_Car.RAPatch_RemoveLock(null), "the lock comes off");
		Check(m_Car.RAPatch_LockState() == RAPatch_VehicleLocks.NONE, "no lock now");
		Check(!m_Car.RAPatch_LockUserAllowed(owner), "and who was allowed is forgotten");

		// Forced open: the lock's lost
		lock = ItemBase.Cast(Spawn(rules.LockItem, Offset(-60, 7)));
		m_Car.RAPatch_FitLock(lock);
		m_Car.RAPatch_SetCode(owner, "1234");
		Check(m_Car.RAPatch_BreakLock(0), "a good roll: it breaks");
		Check(m_Car.RAPatch_LockState() == RAPatch_VehicleLocks.NONE, "the lock's gone");

		// Switched off: a fitted lock does nothing
		lock = ItemBase.Cast(Spawn(rules.LockItem, Offset(-60, 8)));
		m_Car.RAPatch_FitLock(lock);
		m_Car.RAPatch_SetCode(owner, "1234");
		rules.Enabled = false;
		Check(!m_Car.RAPatch_IdIsDenied(stranger), "locks switched off: nobody's shut out");
		rules.Enabled = true;
		Check(m_Car.RAPatch_IdIsDenied(stranger), "switched on again: they are");

		// Placement rows
		RAPatch_VehicleLockSpot spot = new RAPatch_VehicleLockSpot;
		spot.VehicleClass = "SelfTestCar";
		spot.X = 1.25;
		RAPatch_VehicleLockSpot copy = RAPatch_VehicleLockSpot.FromJson(spot.ToJson());
		Check(copy && copy.VehicleClass == "SelfTestCar" && Near(copy.X, 1.25), "a placement row survives the trip to the server");
		RAPatch_Settings.Get().RAPatch_PutSpot(spot);
		RAPatch_Settings.Get().RAPatch_PutSpot(copy);
		int found = 0;
		for (int i = 0; i < rules.Spots.Count(); i++)
		{
			if (rules.Spots[i].VehicleClass == "SelfTestCar")
				found++;
		}
		Check(found == 1, "saving a class twice replaces its row");
		array<ref RAPatch_VehicleLockSpot> savedSpots = rules.Spots;
		rules.Spots = new array<ref RAPatch_VehicleLockSpot>;
		RAPatch_VehicleLockSpot colour = new RAPatch_VehicleLockSpot;
		colour.VehicleClass = "Sedan_02_Grey";
		rules.Spots.Insert(colour);
		Check(RAPatch_VehicleLocks.SpotFor("Sedan_02_Grey") == colour, "a colour finds its own row");
		Check(RAPatch_VehicleLocks.SpotFor("Sedan_02_Red") == colour, "another colour of that car uses it too");
		Check(RAPatch_VehicleLocks.SpotFor("OffroadHatchback_Blue") == null, "an unrelated car doesn't");
		rules.Spots = savedSpots;
		Check(RAPatch_VehicleLocks.SpotFor("SelfTestCar") != null, "and it can be found");
		for (int j = rules.Spots.Count() - 1; j >= 0; j--)
		{
			if (rules.Spots[j].VehicleClass == "SelfTestCar")
				rules.Spots.Remove(j);
		}

		// What clients are sent
		RAPatch_Settings sent = RAPatch_Settings.FromJson(RAPatch_Settings.Get().ToJsonForClients());
		Check(sent && sent.VehicleLocks.LockItem == rules.LockItem, "clients get the lock item");
		Check(sent && sent.VehicleLocks.PlacementAdmins.Count() == 0, "but not who the admins are");
	}

	protected ItemBase BenchItem(string type, vector spot, float quantity = -1)
	{
		ItemBase item = ItemBase.Cast(Spawn(type, spot));
		if (item && quantity >= 0)
			item.SetQuantity(quantity);
		return item;
	}

	protected void CheckBench()
	{
		RAPatch_Bench.LogAvailable();
		array<ref RAPatch_BenchRow> rows = RAPatch_Bench.Rules().Recipes;
		Check(rows.Count() == 36, "the workbench recipes are in the settings: " + rows.Count());

		// Ammo and gunpowder are a straight trade: one full stack for one gunpowder
		foreach (RAPatch_BenchRow check : rows)
		{
			if (check.Results.Count() == 0 || !GetGame().IsKindOf(check.Results[0], "Ammunition_Base") || check.Ingredients.Count() != 1 || check.Ingredients[0].Items[0] != "RA_Gunpowder")
				continue;
			Check(check.Count == 1 && check.QuantityPercent == 100 && check.Ingredients[0].Amount == 1, check.Name + ": 1 gunpowder makes 1 full stack");
		}
		string available = "";
		for (int r = 0; r < rows.Count(); r++)
		{
			if (RAPatch_Bench.IsAvailable(rows[r]))
				available += " " + rows[r].Name + ",";
		}
		Note("available here:" + available);
		RAPatch_BenchRow nails = RAPatch_Bench.RowByName("Nails");
		Check(nails && RAPatch_Bench.IsAvailable(nails), "the nails recipe is available");
		RAPatch_ModuleRow benchRow;
		foreach (RAPatch_ModuleRow module : RAPatch_Settings.Get().Desk.Modules)
		{
			if (module.Workbench)
				benchRow = module;
		}
		if (!benchRow || !nails)
		{
			Note("no workbench module in the settings: the bench isn't tested");
			return;
		}
		m_BenchDesk = TerritoryHQ.Cast(Spawn("TerritoryHQ", Offset(-160, -160)));
		m_BenchDesk.RAPatch_Install(ItemBase.Cast(Spawn(benchRow.Items[0], Offset(-160, -155))));
		Check(m_BenchDesk.RAPatch_HasBench(), "with the module, the desk has a workbench");
		vector pile = Offset(-165, -160);

		// Paying: what's first in the list goes first, as the desk's storage comes before the player
		array<EntityAI> things = new array<EntityAI>;
		ItemBase firstPlates = BenchItem("MetalPlate", pile, 5);
		ItemBase morePlates = BenchItem("MetalPlate", pile, 20);
		things.Insert(firstPlates);
		things.Insert(morePlates);
		Check(m_BenchDesk.RAPatch_BenchStartWith(things, "Nails") != "", "without the hammer: refused");
		Check(morePlates.GetQuantity() == 20, "and nothing's taken");
		ItemBase hammer = BenchItem("Hammer", pile);
		things.Insert(hammer);
		ItemBase ruinedHammer = BenchItem("Hammer", pile);
		ruinedHammer.SetHealth("", "", 0);
		array<EntityAI> justRuined = new array<EntityAI>;
		justRuined.Insert(morePlates);
		justRuined.Insert(ruinedHammer);
		Check(m_BenchDesk.RAPatch_BenchStartWith(justRuined, "Nails") != "", "a ruined hammer doesn't count");
		string started = m_BenchDesk.RAPatch_BenchStartWith(things, "Nails");
		Check(started == "", "with everything: it starts. " + started);
		Check(m_BenchDesk.RAPatch_BenchQueueCount() == 1, "one job in the queue");
		Check(morePlates.GetQuantity() == 17, "the first stack's used up, then 3 of the next: " + morePlates.GetQuantity());

		// Timer and results
		m_BenchDesk.RAPatch_BenchAdvance(nails.Seconds - 1);
		Check(m_BenchDesk.RAPatch_BenchResultCount() == 0, "a second before the end: not ready");
		m_BenchDesk.RAPatch_BenchAdvance(1);
		Check(m_BenchDesk.RAPatch_BenchResultCount() == 1 && m_BenchDesk.RAPatch_BenchQueueCount() == 0, "at the end: it's ready");
		Check(!m_BenchDesk.RAPatch_RemoveNamed(benchRow.Name, null), "the module can't come out with something waiting");
		Check(m_BenchDesk.RAPatch_BenchCollect(null) == 1, "collecting gives it");
		Check(m_BenchDesk.RAPatch_BenchResultCount() == 0, "and it's gone from the desk");

		// Cancelling: the materials come back
		things.Clear();
		things.Insert(morePlates);
		things.Insert(BenchItem("Hammer", pile));
		m_BenchDesk.RAPatch_BenchStartWith(things, "Nails");
		Check(morePlates.GetQuantity() == 9, "another job: 8 more plates");
		Check(m_BenchDesk.RAPatch_BenchCancelLast(null), "it can be cancelled");
		Check(m_BenchDesk.RAPatch_BenchQueueCount() == 0, "and the queue's empty");
		Check(!m_BenchDesk.RAPatch_BenchCancelLast(null), "with nothing to cancel, nothing happens");

		// Queue limit, and several jobs in a row
		int wasQueue = RAPatch_Bench.Rules().QueueSize;
		RAPatch_Bench.Rules().QueueSize = 2;
		for (int job = 0; job < 2; job++)
		{
			things.Clear();
			things.Insert(BenchItem("MetalPlate", pile, 8));
			things.Insert(BenchItem("Hammer", pile));
			Check(m_BenchDesk.RAPatch_BenchStartWith(things, "Nails") == "", "job " + (job + 1) + " goes in");
		}
		things.Clear();
		things.Insert(BenchItem("MetalPlate", pile, 8));
		things.Insert(BenchItem("Hammer", pile));
		Check(m_BenchDesk.RAPatch_BenchStartWith(things, "Nails") != "", "over the limit: refused");
		RAPatch_Bench.Rules().QueueSize = wasQueue;
		m_BenchDesk.RAPatch_BenchAdvance(nails.Seconds + 1);
		Check(m_BenchDesk.RAPatch_BenchQueueCount() == 1 && m_BenchDesk.RAPatch_BenchResultCount() == 1, "the next job starts when one ends");

		// What goes to the player is short
		RAPatch_BenchStatus status = m_BenchDesk.RAPatch_BenchBuildStatus();
		string statusJson;
		JsonSerializer serializer = new JsonSerializer;
		serializer.WriteToString(status, false, statusJson);
		Check(statusJson.Length() < 400, "the menu's message is short enough: " + statusJson);
	}

	// On a desk of their own, far from the rest, so they don't change the first desk's figures
	protected void StartBuildingLimits()
	{
		Check(RAPatch_Limits.KindOf("BaseBuilding_WoodSingleDoor") == RAPatch_Limits.DOOR, "a single door counts as a door");
		Check(RAPatch_Limits.KindOf("BaseBuilding_MetalDoubleDoor") == RAPatch_Limits.DOOR, "so does a double door");
		Check(RAPatch_Limits.KindOf("BaseBuilding_WoodCompoundGate") == RAPatch_Limits.GATE, "a compound gate counts as a gate");
		Check(RAPatch_Limits.KindOf("BaseBuilding_MetalGarageDoor") == RAPatch_Limits.GATE, "so does a garage door");
		Check(RAPatch_Limits.KindOf("BaseBuilding_WoodWall") == RAPatch_Limits.OTHER, "a wall is neither");
		m_LimitDesk = TerritoryHQ.Cast(Spawn("TerritoryHQ", Offset(160, 160)));
		Spawn("BaseBuilding_WoodSingleDoor", Offset(165, 160));
		Spawn("BaseBuilding_WoodCompoundGate", Offset(160, 165));
		Spawn("BaseBuilding_WoodWall", Offset(155, 160));

		// Destroyed: its percentage of the cost, in one place away from everything else
		m_DropSpot = Offset(-160, 160);
		m_DropSpot[1] = GetGame().SurfaceY(m_DropSpot[0], m_DropSpot[2]);
		BaseBuilding doomed = BaseBuilding.Cast(GetGame().CreateObjectEx("BaseBuilding_WoodWall", m_DropSpot, ECE_FULL));
		if (doomed)
		{
			Note("a wood wall costs " + doomed.GetConstructCost().Quantity + " " + doomed.GetConstructCost().Item);
			doomed.SetHealth("GlobalHealth", "Health", 0);
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CheckBuildingLimits, 3000, false);
	}

	protected void CheckBuildingLimits()
	{
		RAPatch_DeskSettings desk = RAPatch_Settings.Get().Desk;
		Check(m_LimitDesk && m_LimitDesk.RAPatch_DoorCount() == 1, "the desk counts its door: " + m_LimitDesk.RAPatch_DoorCount());
		Check(m_LimitDesk && m_LimitDesk.RAPatch_GateCount() == 1, "and its gate: " + m_LimitDesk.RAPatch_GateCount());
		vector inside = Offset(162, 162);
		inside[1] = GetGame().SurfaceY(inside[0], inside[2]);
		Check(RAPatch_Limits.Refusal("BaseBuilding_WoodSingleDoor", inside) == "", "no limit set: a door can go in");
		desk.MaxDoors = 2;
		Check(RAPatch_Limits.Refusal("BaseBuilding_WoodSingleDoor", inside) == "", "1 door of 2: another can go in");
		desk.MaxDoors = 1;
		string full = RAPatch_Limits.Refusal("BaseBuilding_WoodSingleDoor", inside);
		Check(full != "", "1 door of 1: refused. " + full);
		Check(RAPatch_Limits.Refusal("BaseBuilding_WoodWall", inside) == "", "a wall can still go in");
		Check(RAPatch_Limits.Refusal("BaseBuilding_WoodSingleDoor", Offset(-300, -300)) == "", "and a door outside any territory");
		desk.MaxDoors = 0;
		desk.MaxGates = 1;
		Check(RAPatch_Limits.Refusal("BaseBuilding_WoodCompoundGate", inside) != "", "1 gate of 1: refused");
		Check(RAPatch_Limits.Refusal("BaseBuilding_WoodSingleDoor", inside) == "", "the gate limit doesn't touch doors");
		desk.MaxGates = 0;

		// What the destroyed wall dropped
		array<Object> nearby = new array<Object>;
		GetGame().GetObjectsAtPosition3D(m_DropSpot, 12, nearby, null);
		int walls = 0;
		float dropped = 0;
		foreach (Object thing : nearby)
		{
			if (thing.IsKindOf("BaseBuilding_WoodWall"))
				walls++;
			else if (thing.IsKindOf("WoodenPlank"))
			{
				dropped += ItemBase.Cast(thing).GetQuantity();
				GetGame().ObjectDelete(thing);
			}
		}
		Check(walls == 0, "a destroyed wall is gone");
		float wanted = Math.Round(BaseBuilding.GetConstructCost("BaseBuilding_WoodWall").Quantity * RAPatch_Settings.Get().Raiding.DestroyedMaterialsPercent / 100);
		Check(Near(dropped, wanted), "and drops its percentage of the cost: " + dropped + " of the " + wanted + " wanted");

		// The charge log
		RAPatch_ChargeLog.Write("SELF TEST", "nobody", null);
		Check(!RAPatch_Settings.Get().Raiding.LogCharges || FileExist(RAPatch_ChargeLog.PATH), "the charge log can be written");

		// Free placement
		BaseBuilding wall = BaseBuilding.Cast(Spawn("BaseBuilding_WoodWall", Offset(-300, 40)));
		BaseBuilding door = BaseBuilding.Cast(Spawn("BaseBuilding_WoodSingleDoor", Offset(-300, 50)));
		RAPatch_BuildSettings build = RAPatch_Settings.Get().Build;
		bool wasFree = build.FreePlacement;
		build.FreePlacement = true;
		Check((wall.GetAllowedPlacementModes() & PlacementMode.FREE) != 0, "free placement on: a wall can go anywhere");
		Check((door.GetAllowedPlacementModes() & PlacementMode.FREE) == 0, "a door still can't");
		build.FreePlacement = false;
		Check((wall.GetAllowedPlacementModes() & PlacementMode.FREE) == 0, "off: a wall can't");
		build.FreePlacement = wasFree;
	}

	protected void CheckRepair()
	{
		RAPatch_BuildSettings rules = RAPatch_Settings.Get().Build;
		BaseBuilding part = BaseBuilding.Cast(Spawn("BaseBuilding_WoodWall", Offset(-70, 30)));
		Check(part != null, "a wall for the repair check");
		if (!part)
			return;
		float full = part.GetRepairCost().Quantity;
		float maxHealth = part.GetMaxHealth("GlobalHealth", "Health");
		part.SetHealth("GlobalHealth", "Health", maxHealth * 0.99);
		part.SetHealthSynchDirty();
		Note("wall at 99%: synced health " + part.GetSyncedHealth01() + ", full repair cost " + full);
		Check(!part.RAPatch_WantsRepair(), "at 99% health, Repair isn't offered");
		part.SetHealth("GlobalHealth", "Health", maxHealth * 0.9);
		part.SetHealthSynchDirty();
		Check(part.RAPatch_WantsRepair(), "at 90%, it is");
		int nearlyFine = part.GetCalculatedRepairCost().Quantity;
		part.SetHealth("GlobalHealth", "Health", maxHealth * 0.1);
		part.SetHealthSynchDirty();
		int badlyHurt = part.GetCalculatedRepairCost().Quantity;
		Check(badlyHurt > nearlyFine, "a badly damaged part costs more than a lightly damaged one: " + badlyHurt + " against " + nearlyFine);
		Check(badlyHurt <= full, "and never more than the full cost of " + full);
		Check(nearlyFine >= 1, "and always at least 1");
		rules.RepairCostByDamage = false;
		int asRA = part.GetCalculatedRepairCost().Quantity;
		rules.RepairCostByDamage = true;
		Check(asRA < badlyHurt, "switched off, it's RA's own sum: " + asRA);
	}

	protected void CheckCost(string type, string tier)
	{
		int fromConfig = GetGame().ConfigGetInt("CfgVehicles " + type + " UpgradeSystem " + tier + " Construct quantity");
		string itemFromConfig = GetGame().ConfigGetTextOut("CfgVehicles " + type + " UpgradeSystem " + tier + " Construct item");
		BaseBuildingItemEntry cost = BaseBuilding.GetConstructCost(type);
		if (!cost)
		{
			Note(type + " can't be built");
			return;
		}
		Note(type + " costs " + cost.Quantity + " " + cost.Item + ". RA's config: " + fromConfig + " " + itemFromConfig);
		float want = fromConfig;
		string wantItem = itemFromConfig;
		RAPatch_CostRow chosen;
		foreach (RAPatch_CostRow row : RAPatch_Settings.Get().Build.Costs)
		{
			if (row.Part == type)
				chosen = row;
			else if (row.Part == "Tier:" + tier && !chosen)
				chosen = row;
		}
		if (chosen)
		{
			if (chosen.Item != "")
				wantItem = chosen.Item;
			if (chosen.Quantity > 0)
				want = chosen.Quantity;
			want = Math.Round(want * chosen.Multiplier);
		}
		Check(cost.Quantity == want && cost.Item == wantItem, type + " cost matches the file");
	}

	protected void CheckTool(string type)
	{
		ItemBase tool = ItemBase.Cast(Spawn(type, Offset(500, 505)));
		RAPatch_ToolRow row = RAPatch_Tools.Find(tool);
		string answer = "not allowed";
		if (row)
			answer = row.Seconds.ToString() + " s, by the row for " + row.Tool;
		Note("tool " + type + ": " + answer);
	}

	// Real charges against real walls, through RA's own explosion. Several distances from the wall's centre, because
	// RA's blast is only 0.1 m wide and a charge has to be touching.
	protected void StartBlast()
	{
		RAPatch_Settings settings = RAPatch_Settings.Get();
		m_BlastOffsets = {0.05, 0.1, 0.15, 0.2, 0.3, 0.5};
		for (int i = 0; i < m_BlastOffsets.Count(); i++)
		{
			vector wallSpot = Offset(-300, i * 40);
			BaseBuilding wall = BaseBuilding.Cast(Spawn("BaseBuilding_WoodWall", wallSpot));
			m_BlastWalls.Insert(wall);
			RA_C4 charge = RA_C4.Cast(Spawn("RA_C4", wallSpot));
			vector chargeSpot = wall.GetPosition() + Vector(0, 1.5, m_BlastOffsets[i]);
			charge.SetPosition(chargeSpot);
			if (RAPatch_RaidClock.IsOpenNow())
				charge.SetArmState(true);
		}
		RA_C4 sample = RA_C4.Cast(m_Spawned[m_Spawned.Count() - 1]);
		StartDeskBlast();
		Check(Near(sample.GetArmDelay(), settings.Raiding.C4FuseSeconds), "C4 fuse is " + sample.GetArmDelay() + " s");
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CheckBlast, (settings.Raiding.C4FuseSeconds + 3) * 1000, false);
	}

	// Charges on desks: RA's blast skips desks, the addon's doesn't. The desk's top isn't known here, so several heights.
	protected void StartDeskBlast()
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		m_BlastDeskHeights = {0.4, 0.6, 0.75, 0.85, 0.95, 1.1};
		for (int i = 0; i < m_BlastDeskHeights.Count(); i++)
		{
			vector deskSpot = Offset(-340, i * 40);
			TerritoryHQ desk = TerritoryHQ.Cast(Spawn("TerritoryHQ", deskSpot));
			m_BlastDesks.Insert(desk);
			if (!desk)
				continue;
			RA_C4 charge = RA_C4.Cast(Spawn("RA_C4", deskSpot));
			charge.SetPosition(desk.GetPosition() + Vector(0, m_BlastDeskHeights[i], 0));
			if (RAPatch_RaidClock.IsOpenNow())
				charge.SetArmState(true);
		}
		// DeskC4ToDestroy real charges, side by side on one desk's top
		vector threeSpot = Offset(-380, 0);
		m_ThreeC4Desk = TerritoryHQ.Cast(Spawn("TerritoryHQ", threeSpot));
		int count = Math.Ceil(raid.DeskC4ToDestroy - 0.001);
		for (int j = 0; m_ThreeC4Desk && j < count && RAPatch_RaidClock.IsOpenNow(); j++)
		{
			RA_C4 more = RA_C4.Cast(Spawn("RA_C4", threeSpot));
			more.SetPosition(m_ThreeC4Desk.GetPosition() + Vector(-0.3 + j * 0.3, 0.85, 0));
			more.SetArmState(true);
		}
		TerritoryHQ first = m_BlastDesks[0];
		if (!first || raid.C4Damage <= 0 || raid.DeskC4ToDestroy <= 0)
			return;
		float max = first.GetMaxHealth("GlobalHealth", "Health");
		float c4 = ScriptedExplosion.RAPatch_DeskBlastDamage(first, raid.C4Damage);
		float improvised = ScriptedExplosion.RAPatch_DeskBlastDamage(first, raid.ImprovisedDamage);
		int c4Needed = Math.Ceil(max / c4);
		int improvisedNeeded = Math.Ceil(max / improvised);
		Note("desk: " + max + " health, a C4 takes " + c4 + ", an improvised " + improvised + ": " + c4Needed + " C4 or " + improvisedNeeded + " improvised");
		Check(c4Needed == Math.Ceil(raid.DeskC4ToDestroy - 0.001), "DeskC4ToDestroy C4 charges destroy a desk: " + c4Needed);
	}

	protected void CheckDeskBlast()
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		bool safe = !raid.DeskBlastDamage || raid.DoorsOnly || raid.C4Damage <= 0 || raid.DeskC4ToDestroy <= 0;
		int hits = 0;
		for (int i = 0; i < m_BlastDesks.Count(); i++)
		{
			TerritoryHQ desk = m_BlastDesks[i];
			if (!desk)
				continue;
			if (!safe && ScriptedExplosion.RAPatch_IsBlastProof(desk))
				safe = true;
			float full = desk.GetMaxHealth("GlobalHealth", "Health");
			float lost = full - desk.GetHealth("GlobalHealth", "Health");
			Note("desk blast, charge " + m_BlastDeskHeights[i] + " m above the desk's centre: lost " + lost + " of " + full + ", ruined " + desk.IsRuined());
			// Below the desk's top the blast misses. The vanilla explosion alongside is ignored by desks while DeskBlastDamage is on
			Check(lost < 1 || lost > 20, "a charge on a desk either missed or hit, nothing from the vanilla explosion");
			if (lost > 20)
			{
				hits++;
				float want = ScriptedExplosion.RAPatch_DeskBlastDamage(desk, raid.C4Damage);
				if (!safe)
					Check(Math.AbsFloat(lost - want) < 1, "a C4 on a desk took " + want);
			}
		}
		if (!RAPatch_RaidClock.IsOpenNow())
			return;
		if (!safe)
		{
			bool gone = !m_ThreeC4Desk || m_ThreeC4Desk.IsRuined();
			Note("desk with " + raid.DeskC4ToDestroy + " C4 on it: ruined " + gone);
			Check(gone, "DeskC4ToDestroy real C4 charges destroyed a desk");
		}
		if (safe)
			Check(hits == 0, "desks are safe from charges with these settings");
		else
			Check(hits > 0, "at least one charge hit its desk");
	}

	protected void CheckBlast()
	{
		CheckDeskBlast();
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		float want = raid.C4Damage * raid.WoodDamageMultiplier;
		int hits = 0;
		for (int i = 0; i < m_BlastWalls.Count(); i++)
		{
			BaseBuilding wall = m_BlastWalls[i];
			if (!wall)
			{
				Note("blast, charge " + m_BlastOffsets[i] + " m from the wall's centre: wall gone");
				continue;
			}
			float lost = wall.GetMaxHealth("GlobalHealth", "Health") - wall.GetHealth("GlobalHealth", "Health");
			Note("blast, charge " + m_BlastOffsets[i] + " m from the wall's centre: wall lost " + lost + " of " + wall.GetMaxHealth("GlobalHealth", "Health"));
			// RA's blast also makes a small vanilla explosion: about 3 hp. Below this, the blast itself missed the wall.
			if (lost > want * 0.5)
			{
				hits++;
				Check(Near(lost, want), "the blast took the file's damage: " + want);
			}
		}
		if (RAPatch_RaidClock.IsOpenNow() && raid.BlastProofClasses.Count() == 0 && !raid.DoorsOnly)
			Check(hits > 0 || want >= 2500, "at least one charge hit its wall");
		Finish();
	}

	protected void CheckRecipes()
	{
		PluginRecipesManager manager = PluginRecipesManager.Cast(GetPlugin(PluginRecipesManager));
		if (!manager)
		{
			Note("FAIL no recipe manager");
			return;
		}
		array<ref RAPatch_RecipeRow> rows = RAPatch_Settings.Get().Crafting.Recipes;
		int gunpowderId = manager.RecipeIDFromClassname("RAPatch_Recipe0");
		Check(gunpowderId != -1, "our recipe is registered, id " + gunpowderId);

		// The desk kit's numbers
		int kitId = manager.RecipeIDFromClassname("CraftTerritoryHQKit");
		RecipeBase kit = manager.m_RecipeList[kitId];
		RAPatch_RecipeRow kitRow = RAPatch_Settings.Get().Crafting.DeskKit;
		Check(Near(kit.m_MinQuantityIngredient[1], kitRow.Ingredient2Amount), "desk kit needs " + kit.m_MinQuantityIngredient[1] + " sticks");

		if (rows.Count() == 0 || !rows[0].Enabled)
		{
			Note("recipe 0 is switched off: not crafted");
			return;
		}
		RAPatch_RecipeRow row = rows[0];
		// Real items, on the ground beside the test character, through the game's own recipe code
		m_Player.GetInventory().CreateInInventory("HuntingJacket_Brown");
		// The game only offers a recipe when one ingredient is in hand
		vector spot = m_Player.GetPosition() + "1 0 0";
		ItemBase first = ItemBase.Cast(m_Player.GetHumanInventory().CreateInHands(row.Ingredient1[0]));
		Check(first != null, row.Ingredient1[0] + " is in the test character's hands");
		if (!first)
			return;
		string secondType = row.Ingredient2[0];
		if (secondType == "Ammunition_Base")
			secondType = "Ammo_9x19";
		ItemBase second = ItemBase.Cast(Spawn(secondType, spot));
		float amountBefore = 0;
		Magazine pile = Magazine.Cast(second);
		if (pile)
		{
			pile.ServerSetAmmoCount(pile.GetAmmoMax());
			amountBefore = pile.GetAmmoCount();
		}
		TStringArray path = new TStringArray;
		GetGame().ConfigGetFullPath("CfgMagazines " + secondType, path);
		Note(secondType + " inherits from: " + path.Count() + " levels, top " + path[path.Count() - 1]);
		foreach (string level : path)
			Note("  " + level);
		array<int> found = new array<int>;
		manager.GetValidRecipes(first, second, found, m_Player);
		if (row.NeedsModule != "")
		{
			Check(found.Find(gunpowderId) == -1, "far from the desk, a recipe that needs " + row.NeedsModule + " isn't offered");
			vector beside = m_Desk.GetPosition() + "2 0 0";
			m_Player.SetPosition(beside);
			second.SetPosition(beside);
			manager.GetValidRecipes(first, second, found, m_Player);
		}
		string ids = "";
		foreach (int id : found)
			ids += " " + id;
		if (pile && row.Ingredient2MustBeFull)
		{
			pile.ServerSetAmmoCount(pile.GetAmmoMax() - 1);
			array<int> notFull = new array<int>;
			manager.GetValidRecipes(first, second, notFull, m_Player);
			Check(notFull.Find(gunpowderId) == -1, "one round short of a full stack: not offered");
			pile.ServerSetAmmoCount(pile.GetAmmoMax());
			manager.GetValidRecipes(first, second, found, m_Player);
		}
		Check(found.Find(gunpowderId) != -1, "the game offers our recipe for " + first.GetType() + " + " + second.GetType() + ". Offered:" + ids);
		if (found.Find(gunpowderId) == -1)
			return;
		manager.PerformRecipeServer(gunpowderId, first, second, m_Player);
		if (pile && row.Ingredient2MustBeFull)
		{
			Check(!pile || pile.IsSetForDeletion(), "crafting took the whole stack");
		}
		else if (pile)
		{
			float amountAfter = pile.GetAmmoCount();
			Note("crafted: ammo went from " + amountBefore + " to " + amountAfter + ", the file says it costs " + row.Ingredient2Amount);
			Check(Near(amountBefore - amountAfter, row.Ingredient2Amount), "crafting took the file's cost");
		}
		array<EntityAI> carried = new array<EntityAI>;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, carried);
		ItemBase result;
		foreach (EntityAI thing : carried)
		{
			if (thing.GetType() == row.Result)
				result = ItemBase.Cast(thing);
		}
		Check(result != null, "crafting made a " + row.Result);
		if (result)
		{
			if (row.ResultQuantity >= 0)
				Check(Near(result.GetQuantity(), row.ResultQuantity), "with quantity " + result.GetQuantity());
		}
	}

	protected void Finish()
	{
		if (RAPatch_Settings.Get().Desk.Modules.Count() > 0 && !RAPatch_Settings.Get().SelfTestKeepDesk)
		{
			CheckRemoval();
			return;
		}
		FinishCleanup();
	}

	protected void FinishCleanup()
	{
		bool keep = RAPatch_Settings.Get().SelfTestKeepDesk;
		foreach (Object thing : m_Spawned)
		{
			if (thing && !(keep && (thing == m_Desk || thing == m_Car || thing == m_BenchDesk)))
				GetGame().ObjectDelete(thing);
		}
		Note("=== done: " + m_Checks + " checks, " + m_Failures + " failed");
		// An idle server doesn't save, so a forced stop would lose the desk. Shutting down properly saves the world.
		if (keep)
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GetGame().RequestExit, 5000, false, 0);
	}
}
