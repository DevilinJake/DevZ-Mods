// The admin's file: profiles\DevZ_RAAddon\Settings.json, written with these defaults on first run.
// Most defaults are RA's own numbers. The additions are marked Proposed: crafting recipes, and respawning at bags and desks.
// Also by default: only a territory's team can deploy in it, which RA nearly does already.

class RAPatch_DecaySettings
{
	bool Enabled = true;			// false: no part, desk or container loses health, and no nails are used
	float FrameHours = 2;			// hours for a part with no upkeep to go from full health to gone
	float WoodHours = 12;
	float MetalHours = 18;
	float ConcreteHours = 24;
	float DeskHours = 24;
	float ContainerHours = 24;		// tents, barrels, crates
	float PartNailsPerDay = 0.5;	// each part in the desk's radius
	float LadderNailsPerDay = 0.001;
	float DeskNailsPerDay = 50;
	float ContainerNailsPerDay = 0.00001;
}

// A desk module: any item class, named here. Set how players find it in your own loot tables or trader.
class RAPatch_ModuleRow
{
	string Name;			// shown to players, and what a recipe's NeedsModule refers to
	ref array<string> Items = new array<string>;	// any of these classes, or anything inheriting from them, counts as this module
	float RadiusAdded;		// metres
	int PlayersAdded;		// when MaxPlayers isn't 0
	float UpkeepMultiplier = 1;	// 0.5 = half the nails
	bool Storage;			// opens the desk's storage space, which is a fixed size set in the mod's config
	bool Workbench;			// adds "Open Workbench" to the desk. Recipes are in Crafting.Bench
	bool RaidAlarm;			// the desk's team can add a Discord webhook on the dashboard, for alerts when an enemy damages the base

	void RAPatch_ModuleRow(string name = "", string item = "")
	{
		Name = name;
		if (item != "")
			Items.Insert(item);
	}
}

class RAPatch_DeskSettings
{
	float Radius = 36;				// metres, in 3D
	int MaxPlayers = 0;				// 0 = no limit
	int MaxDoors = 8;				// single and double doors per territory. 0 = no limit
	int MaxGates = 2;				// compound gates and garage doors per territory. 0 = no limit
	bool OpenAuthorization = true;	// false: on a desk with no locked code lock, only the first player can authorise
	ref array<ref RAPatch_ModuleRow> Modules = new array<ref RAPatch_ModuleRow>;	// up to 16. Empty = no modules
	float ModuleInstallSeconds = 5;
	bool ModulesRemovable = true;	// true: authorised players can take a module back out. false: it stays until the desk is gone
	bool RadiusMayOverlap = false;	// false: territories can't overlap. A desk can't be placed where its territory would, and a radius module that would is refused
	float ModuleCraftDistance = 5;	// how close to the desk a recipe's NeedsModule counts, in metres
	int DeployRule = 0;					// deploying inside a territory. 0 only its team, 1 anyone, 2 only its team, except traps and mines. RA's charges can always be placed
	bool DeployChecksPlacedSpot = true;	// RA checks where the player stands. This also checks where the object lands
	bool DashboardEnabled = true;	// authorised players can open the HQ dashboard on a desk
	float RaidAlarmCooldownSeconds = 300;	// the most often a desk posts. Hits in between are counted in the next message
}

// Where a dead player comes back: a bag they've set, then a desk they're authorised on, then vanilla's spawn points.
// Switched off or on cooldown passes to the next. The default numbers are ours.
class RAPatch_RespawnSettings
{
	bool BagsEnabled = true;
	bool HQEnabled = true;
	bool DeskWhenNoBag = false;	// false: with bags on, a bag is the only way back, and no bag means vanilla's spawn points. true: no working bag, the desk instead
	ref array<string> BagClasses = new array<string>;	// laid-out objects that count as a sleeping bag. Any mod's class, or anything inheriting from it
	float BagCooldownSeconds = 180;
	float HQCooldownSeconds = 180;
	float CooldownPenaltySeconds = 120;			// respawn again within the window, and the next cooldown is this much longer. It adds up. 0 = off
	float CooldownPenaltyWindowSeconds = 120;	// how long after a cooldown ends a respawn still counts. Wait longer, and it's back to the base cooldown
	float CooldownMaxSeconds = 7200;			// the longest a cooldown can grow to. 0 = no limit
	float RespawnWaterPercent = 50;				// water and energy to come back with at a bag or desk. Health and blood are full
	float RespawnEnergyPercent = 50;
	float BagWearPercent = 10;			// of the bag's health, per respawn on it. It's gone when it's ruined
	bool BagsNeedOwnTerritory = true;	// true: a bag can only be laid out inside a territory the player is authorised on, and only works while it stays in one
	int BagPickup = 2;					// this mod's own bag only: another mod's bag has its own pack action. 0 anyone, 1 only who laid it out, 2 any team member
	float BagLifetimeHours = 336;		// how long a laid-out bag lasts
}

// Where the code lock is drawn on one class of car. An admin sets these with the placement tool.
class RAPatch_VehicleLockSpot
{
	string VehicleClass;
	float X;
	float Y;
	float Z;
	float Yaw;
	float Pitch;
	float Roll;

	string ToJson()
	{
		string text;
		string error;
		JsonFileLoader<RAPatch_VehicleLockSpot>.MakeData(this, text, error, false);
		return text;
	}

	static RAPatch_VehicleLockSpot FromJson(string text)
	{
		RAPatch_VehicleLockSpot spot = new RAPatch_VehicleLockSpot;
		string error;
		if (!JsonFileLoader<RAPatch_VehicleLockSpot>.LoadData(text, spot, error))
			return null;
		return spot;
	}
}

class RAPatch_VehicleLockSettings
{
	bool Enabled = true;			// false: no car can be locked, and any fitted lock does nothing. Turn it off beside another mod's car locks or keys
	string LockItem = "RA_CodeLock";	// what's fitted. Any item class, or anything inheriting from it
	string RemoveTool = "Pliers";	// takes the lock off, for someone who has entered the code. The item comes back
	float RemoveSeconds = 15;
	string BreakTool = "Lockpick";	// forces a locked car. Empty = it can't be forced
	float BreakSeconds = 60;
	float BreakChance = 0.3;		// from 0 to 1. If it works, the lock's lost
	float BreakToolWearPercent = 34;
	bool AlarmOnBreak = true;		// the car\'s horn pulses while someone\'s forcing it
	float WrongCodeWaitSeconds = 5;
	ref array<string> PlacementAdmins = new array<string>;	// Steam64 IDs who get the placement tool
	ref array<ref RAPatch_VehicleLockSpot> Spots = new array<ref RAPatch_VehicleLockSpot>;	// written by the placement tool
}

// A row's Part is a part's class name, or Tier:Frame, Tier:Wood, Tier:Metal, Tier:Concrete, or Tier:All. The closest match wins.
class RAPatch_CostRow
{
	string Part;
	string Item;			// empty = RA's material
	int Quantity;			// 0 = RA's amount, -1 = this part can't be built or upgraded to
	float Multiplier = 1;	// applied after Quantity

	void RAPatch_CostRow(string cls = "", string item = "", int quantity = 0, float multiplier = 1)
	{
		Part = cls;
		Item = item;
		Quantity = quantity;
		Multiplier = multiplier;
	}
}

class RAPatch_ToolRow
{
	string Tool;	// the first row whose class the tool is, or inherits from, is the one that counts
	float Seconds;

	void RAPatch_ToolRow(string cls = "", float seconds = 0)
	{
		Tool = cls;
		Seconds = seconds;
	}
}

class RAPatch_ZoneRow
{
	float X;
	float Z;
	float Radius;
}

class RAPatch_BuildSettings
{
	ref array<ref RAPatch_CostRow> Costs = new array<ref RAPatch_CostRow>;
	float RepairCostMultiplier = 1;
	bool FreePlacement = false;		// true: parts that only snap can also be placed anywhere, so a wall can stand on its own, even in mid-air. Doors and gates still need a frame
	float SlowRotationMultiplier = 0.2;	// hold Shift while turning a part to turn this much slower. 1 = no difference
	float RepairBelowPercent = 95;	// Repair is only offered below this much health, so tiny decay doesn't offer a paid repair. 100 = any damage, as RA has it
	bool RepairCostByDamage = true;	// true: pay for the damage, so a badly damaged part costs the most. false: RA's own sum, full cost times the health it still has
	ref array<ref RAPatch_ToolRow> UpgradeTools = new array<ref RAPatch_ToolRow>;	// tools that upgrade and repair. RA_ClawHammer (its real items are RA_ClawHammer_Blue, _Red, _Yellow), Hammer, Hatchet and SledgeHammer can be listed
	float HeightLimitMeters = 25;	// above the ground
	bool BuildAnywhere = false;		// true: parts ignore other objects when placed. Snapping rules still apply
	bool ServerChecksSpot = false;	// true: the server repeats the height and no-build checks on the placed spot, not just the client
	ref array<ref RAPatch_ZoneRow> NoBuildZones = new array<ref RAPatch_ZoneRow>;
	bool BlueprintFromPaper = true;
	float BlueprintSeconds = 3;
}

class RAPatch_RaidWindow
{
	string Day;		// Mon Tue Wed Thu Fri Sat Sun
	string Start;	// HH:MM
	string End;

	void RAPatch_RaidWindow(string day = "", string start = "", string end = "")
	{
		Day = day;
		Start = start;
		End = end;
	}
}

class RAPatch_RaidSettings
{
	float C4Damage = 1275;
	float ImprovisedDamage = 750;
	float DestroyedMaterialsPercent = 25;	// of a part's cost, dropped when it's destroyed. 0 = nothing. -1 = RA's own: a coin flip, then a random amount
	bool LogCharges = true;			// who arms and disarms charges, in profiles\DevZ_RAAddon\ChargeLog.txt. Never sent to players
	float C4FuseSeconds = 58;
	float ImprovisedFuseSeconds = 58;
	float BlastRadiusMeters = 0.1;		// how far the blast looks for parts. RA's is very short: the charge must touch the part
	int PenetrationDepth = 0;			// how many parts the blast goes through
	float PenetrationFalloff = 0.65;	// damage is multiplied by this per part it went through
	string C4PlayerAmmo = "C4_Explosive_Ammo";	// the vanilla explosion made for players and sound. Plastic_Explosive_Ammo hurts more
	string ImprovisedPlayerAmmo = "Improvised_Explosive_Ammo";
	float FrameDamageMultiplier = 1;	// the blast's damage against each tier
	float WoodDamageMultiplier = 1;
	float MetalDamageMultiplier = 1;
	float ConcreteDamageMultiplier = 1;
	bool DeskBlastDamage = false;		// true: charges can destroy the desk. RA's own blast skips desks; this lets it hit them. false: charges can't hurt the desk
	float DeskC4ToDestroy = 3;			// with DeskBlastDamage on: C4 charges that destroy a desk, whatever C4Damage is. Improvised charges take their share by their damage against C4Damage: 6 at 750 and 1275
	bool DoorsOnly = false;				// true: only doors, gates, hatches, windows and shutters take blast damage
	ref array<string> BlastProofClasses = new array<string>;	// these classes, and any inheriting from them, take no blast damage
	bool MeleeDamage = true;			// false: melee can't hurt parts
	bool BulletDamage = true;
	bool OtherExplosiveDamage = true;	// grenades and vanilla explosives, not RA's charges
	bool RaidHoursEnabled = false;		// true: charges can only be armed in the windows below. A lit fuse still finishes
	float RaidHoursUtcOffset = 10;
	ref array<ref RAPatch_RaidWindow> RaidWindows = new array<ref RAPatch_RaidWindow>;
}

// For the server's staff: every raid on every base, posted to the admin's own Discord webhook, whether or not the base has
// a raid alarm. This section stays on the server: it's never sent to players, and the address is never logged.
class RAPatch_AdminAlertSettings
{
	string Webhook = "";			// a Discord webhook address for a staff channel. Empty = off
	float CooldownSeconds = 300;	// the most often one base posts. Hits in between are counted in its next message
	bool ShowAttacker = true;		// the attacker's name and Steam64 ID, when a player made the hit. RA's charges don't say who armed them
}

class RAPatch_DismantleSettings
{
	bool ToolRemoveEnabled = true;		// the hammer or hatchet's Remove on RA's parts. It always needs desk permission, or no desk in range
	float RemoveWindowSeconds = 600;	// how long after being placed or upgraded a part can be removed or rotated. Doors and windows with no lock: always
	float VanillaSecondsAuthorized = 5;		// vanilla parts (a watchtower) with permission
	float VanillaSecondsOutsider = 300;		// and without. RA's two BASE_DISMANTLE numbers
	bool VanillaOutsidersAllowed = true;
	int DeskPackUp = 1;					// the screwdriver or pliers' "Dismantle into box" on a desk with no lock. 0 = anyone (RA), 1 = authorised players only, 2 = nobody
	float DeskPackUpSeconds = 2.5;
}

// One crafting recipe. Quantities are from the item's stack: rounds in an ammo pile, percent for duct tape.
class RAPatch_RecipeRow
{
	string Name;
	bool Enabled;
	ref array<string> Ingredient1 = new array<string>;	// any one of these classes, or anything inheriting from it
	float Ingredient1Amount;	// 0 = it only has to be there
	bool Ingredient1Consumed;	// true: Amount is taken, or the whole item when Amount is 0
	bool Ingredient1MustBeFull;	// true: only a full stack counts
	ref array<string> Ingredient2 = new array<string>;
	float Ingredient2Amount;
	bool Ingredient2Consumed;
	bool Ingredient2MustBeFull;
	string Result;
	float ResultQuantity = -1;	// -1 = as it comes
	bool ResultOnGround;		// true: always on the ground. false: in the crafter's inventory if it fits
	string NeedsModule;			// a module's Name: the crafter must be near a desk they're authorised on with it installed
	float Seconds = 4;

	void RAPatch_RecipeRow(string name = "", bool enabled = false, string result = "", float resultQuantity = -1, float seconds = 4)
	{
		Name = name;
		Enabled = enabled;
		Result = result;
		ResultQuantity = resultQuantity;
		Seconds = seconds;
	}
}

// One ingredient on the workbench: any one of these classes, or anything inheriting from it
class RAPatch_BenchItem
{
	ref array<string> Items = new array<string>;
	float Amount = 1;
	bool Consumed = true;	// false: it only has to be there, like a tool
}

class RAPatch_BenchRow
{
	string Name;
	bool Enabled = true;
	int Seconds = 60;
	ref array<ref RAPatch_BenchItem> Ingredients = new array<ref RAPatch_BenchItem>;
	ref array<string> Results = new array<string>;	// one of these, at random
	int Count = 1;					// how many items it makes
	float QuantityPercent = 100;	// how full each stack is

	void RAPatch_BenchRow(string name = "", int seconds = 60, int count = 1, float percent = 100)
	{
		Name = name;
		Seconds = seconds;
		Count = count;
		QuantityPercent = percent;
	}

	void Need(string item, float amount)
	{
		RAPatch_BenchItem part = new RAPatch_BenchItem;
		part.Items.Insert(item);
		part.Amount = amount;
		Ingredients.Insert(part);
	}
}

class RAPatch_BenchSettings
{
	int QueueSize = 5;		// jobs waiting on one desk, including the one being made
	int MaxWaiting = 20;	// finished items a desk can hold before a new job is refused
	ref array<ref RAPatch_BenchRow> Recipes = new array<ref RAPatch_BenchRow>;	// a recipe for items this server doesn't have doesn't show
}

class RAPatch_CraftSettings
{
	ref RAPatch_BenchSettings Bench = new RAPatch_BenchSettings;
	ref RAPatch_RecipeRow DeskKit;		// RA's own recipe for the desk. Its result can't change
	ref array<ref RAPatch_RecipeRow> Recipes = new array<ref RAPatch_RecipeRow>;	// up to 12: every client has to register them before it can hear our settings
}

class RAPatch_Settings
{
	int Version = 17;

	ref RAPatch_DecaySettings Decay = new RAPatch_DecaySettings;
	ref RAPatch_DeskSettings Desk = new RAPatch_DeskSettings;
	ref RAPatch_BuildSettings Build = new RAPatch_BuildSettings;
	ref RAPatch_RaidSettings Raiding = new RAPatch_RaidSettings;
	ref RAPatch_DismantleSettings Dismantling = new RAPatch_DismantleSettings;
	ref RAPatch_CraftSettings Crafting = new RAPatch_CraftSettings;
	ref RAPatch_RespawnSettings Respawn = new RAPatch_RespawnSettings;
	ref RAPatch_VehicleLockSettings VehicleLocks = new RAPatch_VehicleLockSettings;
	ref RAPatch_AdminAlertSettings AdminAlerts = new RAPatch_AdminAlertSettings;
	bool SelfTest = false;	// true: on boot, spawn test parts at SelfTestX, SelfTestZ, check them against this file, and write SelfTest.txt
	float SelfTestX = 7500;
	float SelfTestZ = 7500;
	bool SelfTestKeepDesk = false;	// true: leave the desk and its modules in place, to check they survive a restart

	static const int CURRENT_VERSION = 17;
	static const int MAX_RECIPES = 12;
	static const int MAX_MODULES = 16;
	static const string FOLDER = "$profile:DevZ_RAAddon";
	static const string PATH = "$profile:DevZ_RAAddon/Settings.json";

	protected static ref RAPatch_Settings s_Instance;
	static ref ScriptInvoker OnReplaced = new ScriptInvoker;	// a client got the server's settings

	static RAPatch_Settings Get()
	{
		if (!s_Instance)
		{
			#ifdef SERVER
			s_Instance = Load();
			#else
			s_Instance = new RAPatch_Settings;
			s_Instance.FillDefaults();
			#endif
		}
		return s_Instance;
	}

	// A client, on the server's values
	static void Replace(RAPatch_Settings settings)
	{
		s_Instance = settings;
		OnReplaced.Invoke();
	}

	protected static RAPatch_Settings Load()
	{
		MakeDirectory(FOLDER);
		RAPatch_Settings settings = new RAPatch_Settings;
		string error;
		if (!FileExist(PATH))
		{
			settings.FillDefaults();
			JsonFileLoader<RAPatch_Settings>.SaveFile(PATH, settings, error);
			Log("first run, wrote the defaults to " + PATH);
		}
		else if (!JsonFileLoader<RAPatch_Settings>.LoadFile(PATH, settings, error))
		{
			Log("WARNING could not read the file, using defaults for this run: " + error);
			settings = new RAPatch_Settings;
			settings.FillDefaults();
		}
		else
		{
			Log("read " + PATH);
		}
		settings.Validate();
		settings.Upgrade();
		return settings;
	}

	static void Log(string message)
	{
		PrintString("[DevZ_RAAddon] " + message);
	}

	// An older file has just been read, with defaults for anything it didn't have. Writing it back gives the
	// admin the new settings to edit, and keeps every value they'd set.
	protected void Upgrade()
	{
		if (Version >= CURRENT_VERSION)
			return;
		int oldVersion = Version;
		if (oldVersion < 11)
		{
			if (Crafting.Bench.Recipes.Count() == 0)
				FillBenchRecipes();
			for (int m = 0; m < Desk.Modules.Count(); m++)
			{
				if (Desk.Modules[m].Name == "Workbench")
					Desk.Modules[m].Workbench = true;
			}
		}
		if (oldVersion < 13)
		{
			// Ammo and gunpowder became a straight trade. Left alone if an admin has changed them.
			foreach (RAPatch_RecipeRow hand : Crafting.Recipes)
			{
				if (hand.Result == "RA_Gunpowder" && hand.Ingredient2Amount == 20)
				{
					hand.Ingredient2Amount = 0;
					hand.Ingredient2MustBeFull = true;
				}
			}
			foreach (RAPatch_BenchRow ammoRow : Crafting.Bench.Recipes)
			{
				if (ammoRow.QuantityPercent == 50 && ammoRow.Ingredients.Count() == 1 && ammoRow.Ingredients[0].Amount == 2)
				{
					ammoRow.QuantityPercent = 100;
					ammoRow.Ingredients[0].Amount = 1;
				}
			}
		}
		if (oldVersion < 12)
		{
			// RA's numbers made ammo from nothing with our gunpowder recipe. Left alone if an admin has changed them.
			array<ref RAPatch_BenchRow> rows = Crafting.Bench.Recipes;
			for (int r = rows.Count() - 1; r >= 0; r--)
			{
				if ((rows[r].Name == ".308 Win" || rows[r].Name == "5.56x45mm") && rows[r].Count == 2 && rows[r].Ingredients.Count() == 1 && rows[r].Ingredients[0].Amount == 1)
					rows.Remove(r);
			}
			FillAmmoRecipes();
		}
		if (oldVersion < 4 && Respawn.BagClasses.Count() == 0)
			Respawn.BagClasses.Insert("RAPatch_SleepingBagDeployed_Base");
		Version = CURRENT_VERSION;
		string error;
		JsonFileLoader<RAPatch_Settings>.SaveFile(PATH, this, error);
		Log("upgraded the settings file from version " + oldVersion + " to " + CURRENT_VERSION);
	}

	// A straight trade with the hand recipe for gunpowder, which takes one full stack of any ammo: this can't make ammo from nothing.
	protected void AddAmmoRecipe(string name, string ammo)
	{
		if (RAPatch_FindBenchRow(name))
			return;
		RAPatch_BenchRow row = new RAPatch_BenchRow(name, 40, 1, 100);
		row.Results.Insert(ammo);
		row.Need("RA_Gunpowder", 1);
		Crafting.Bench.Recipes.Insert(row);
	}

	protected RAPatch_BenchRow RAPatch_FindBenchRow(string name)
	{
		array<ref RAPatch_BenchRow> rows = Crafting.Bench.Recipes;
		for (int i = 0; i < rows.Count(); i++)
		{
			if (rows[i].Name == name)
				return rows[i];
		}
		return null;
	}

	protected void FillAmmoRecipes()
	{
		AddAmmoRecipe("12ga Pellets", "Ammo_12gaPellets");
		AddAmmoRecipe("12ga Slug", "Ammo_12gaSlug");
		AddAmmoRecipe("12ga Rubber Slug", "Ammo_12gaRubberSlug");
		AddAmmoRecipe("12ga Beanbag", "Ammo_12gaBeanbag");
		AddAmmoRecipe(".22", "Ammo_22");
		AddAmmoRecipe(".308 Win", "Ammo_308Win");
		AddAmmoRecipe(".308 Win Tracer", "Ammo_308WinTracer");
		AddAmmoRecipe(".357", "Ammo_357");
		AddAmmoRecipe(".380", "Ammo_380");
		AddAmmoRecipe(".45 ACP", "Ammo_45ACP");
		AddAmmoRecipe("5.45x39mm", "Ammo_545x39");
		AddAmmoRecipe("5.45x39mm Tracer", "Ammo_545x39Tracer");
		AddAmmoRecipe("5.56x45mm", "Ammo_556x45");
		AddAmmoRecipe("5.56x45mm Tracer", "Ammo_556x45Tracer");
		AddAmmoRecipe("7.62x39mm", "Ammo_762x39");
		AddAmmoRecipe("7.62x39mm Tracer", "Ammo_762x39Tracer");
		AddAmmoRecipe("7.62x54mmR", "Ammo_762x54");
		AddAmmoRecipe("7.62x54mmR Tracer", "Ammo_762x54Tracer");
		AddAmmoRecipe("9x19mm", "Ammo_9x19");
		AddAmmoRecipe("9x39mm", "Ammo_9x39");
		AddAmmoRecipe("9x39mm AP", "Ammo_9x39AP");
		AddAmmoRecipe("Hunting Bolts", "Ammo_HuntingBolt");
	}

	// RA's own workbench recipes, as data, with its ammo numbers changed: see AddAmmoRecipe
	protected void FillBenchRecipes()
	{
		FillAmmoRecipes();
		RAPatch_BenchRow row;
		row = new RAPatch_BenchRow("Plastic Explosives", 600, 1, 100);
		row.Results.Insert("RA_C4");
		row.Need("RA_Gunpowder", 20);
		row.Need("RemoteDetonator", 1);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Improvised C4", 150, 1, 100);
		row.Results.Insert("RA_Improvised_C4");
		row.Need("RA_Gunpowder", 10);
		row.Need("PersonalRadio", 1);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("RPG7 Rocket", 300, 1, 100);
		row.Results.Insert("RA_Ammo_RPG7");
		row.Need("RA_Gunpowder", 12);
		row.Need("MetalPlate", 5);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("AH6 Rocket", 300, 1, 100);
		row.Results.Insert("RA_AH6_Rocket");
		row.Need("RA_Gunpowder", 5);
		row.Need("MetalPlate", 8);
		row.Need("M67Grenade", 1);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Nails", 60, 1, 100);
		row.Results.Insert("Nail");
		row.Need("MetalPlate", 8);
		row.Need("Hammer", 1);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow(".408 Cheytac Ammo", 300, 10, 100);
		row.Results.Insert("RA_Ammo_408");
		row.Need("RA_Gunpowder", 1);
		row.Need("RA_Ammo_338lm", 10);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow(".50 Cal Ammo", 300, 10, 100);
		row.Results.Insert("RA_Ammo_127x99");
		row.Need("RA_Gunpowder", 1);
		row.Need("RA_Ammo_338lm", 10);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Explosive Crossbow Bolt", 60, 1, 100);
		row.Results.Insert("Ammo_ExplosiveBolt");
		row.Need("RA_Gunpowder", 1);
		row.Need("Ammo_HuntingBolt", 1);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Bed", 120, 1, 100);
		row.Results.Insert("RA_Bed_Kit");
		row.Need("Fabric", 15);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Tarp", 20, 1, 100);
		row.Results.Insert("Fabric");
		row.Need("RA_SleepingBag_Base_Kit", 1);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Sleeping Bag", 30, 1, 100);
		row.Results.Insert("RA_SleepingBag_Green_Kit");
		row.Results.Insert("RA_SleepingBag_Blue_Kit");
		row.Results.Insert("RA_SleepingBag_Yellow_Kit");
		row.Results.Insert("RA_SleepingBag_Red_Kit");
		row.Results.Insert("RA_SleepingBag_Purple_Kit");
		row.Need("Fabric", 5);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Propital Injector", 30, 1, 100);
		row.Results.Insert("RA_Propital");
		row.Need("GlassBottle", 500);
		row.Need("RA_IFAK_Base", 2);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("ETG-C Injector", 30, 1, 100);
		row.Results.Insert("RA_ETGC");
		row.Need("JackDaniels", 500);
		row.Need("RA_Grizzly", 5);
		Crafting.Bench.Recipes.Insert(row);
		row = new RAPatch_BenchRow("Adrenaline Injector", 30, 1, 100);
		row.Results.Insert("RA_Adrenaline");
		row.Need("RA_ALU_Splint", 1);
		row.Need("RA_GoldenStar", 4);
		Crafting.Bench.Recipes.Insert(row);
	}

	protected void AddSpot(string vehicleClass, float x, float y, float z, float yaw, float pitch, float roll)
	{
		RAPatch_VehicleLockSpot spot = new RAPatch_VehicleLockSpot;
		spot.VehicleClass = vehicleClass;
		spot.X = x;
		spot.Y = y;
		spot.Z = z;
		spot.Yaw = yaw;
		spot.Pitch = pitch;
		spot.Roll = roll;
		VehicleLocks.Spots.Insert(spot);
	}

	void FillDefaults()
	{
		FillBenchRecipes();
		// Placed by hand in-game. Other colours use these too; a row for a class that isn't on the server does nothing
		AddSpot("UAZ_31514_hunter", -0.9091, 1.0815, 0.1746, 90, 0, 0);
		AddSpot("UAZ_452_offroad_raid_camor", -1.0403, 1.4233, 0.8129, 90, 0, 0);
		AddSpot("OffroadHatchback", -0.8470, 1.0869, -0.6043, 90, -6, -2);
		AddSpot("Hatchback_02_Black", -0.7955, 0.7978, 0.9534, 90, -8, 0);
		AddSpot("Truck_01_Covered", -0.8636, 1.0972, 0.7729, 95, 0, 0);
		AddSpot("Sedan_02_Grey", -0.8017, 0.7904, 0.9963, 93, -8, 0);
		AddSpot("CivilianSedan", -0.9293, 0.9012, 1.1024, 90, -11, 0);
		AddSpot("Offroad_02", -1.0905, 0.8750, 0.8522, 90, 0, 0);
		AddSpot("UAZ_452", -1.0374, 1.2176, 0.7929, 90, -1, 0);
		AddSpot("UAZ_452_offroad_exp", -1.0363, 1.7179, 0.8179, 90, -2, 0);

		Build.UpgradeTools.Insert(new RAPatch_ToolRow("RA_ClawHammer", 1));
		Build.UpgradeTools.Insert(new RAPatch_ToolRow("Hatchet", 6));
		Build.UpgradeTools.Insert(new RAPatch_ToolRow("Hammer", 3));

		RAPatch_RecipeRow kit = new RAPatch_RecipeRow("", true, "TerritoryHQ_Kit", -1, 2);
		kit.Ingredient1.Insert("Rope");
		kit.Ingredient1Amount = 1;
		kit.Ingredient1Consumed = true;
		kit.Ingredient2.Insert("WoodenStick");
		kit.Ingredient2.Insert("Ammo_SharpStick");
		kit.Ingredient2Amount = 3;
		kit.Ingredient2Consumed = true;
		kit.ResultOnGround = true;
		Crafting.DeskKit = kit;

		// Ours, not RA's. Gunpowder's own description promises crafting; RA has no recipe for it.
		RAPatch_RecipeRow gunpowder = new RAPatch_RecipeRow("Make gunpowder", true, "RA_Gunpowder", 1, 8);
		gunpowder.Ingredient1.Insert("Pliers");
		gunpowder.Ingredient2.Insert("Ammunition_Base");
		gunpowder.Ingredient2Consumed = true;	// a straight trade with the workbench's ammo: one full stack of anything for one gunpowder
		gunpowder.Ingredient2MustBeFull = true;
		Crafting.Recipes.Insert(gunpowder);

		RAPatch_RecipeRow improvised = new RAPatch_RecipeRow("Make improvised charge", false, "RA_Improvised_C4", -1, 12);
		improvised.Ingredient1.Insert("RA_Gunpowder");
		improvised.Ingredient1Amount = 5;
		improvised.Ingredient1Consumed = true;
		improvised.Ingredient2.Insert("DuctTape");
		improvised.Ingredient2Amount = 25;
		improvised.Ingredient2Consumed = true;
		Crafting.Recipes.Insert(improvised);

		Respawn.BagClasses.Insert("RAPatch_SleepingBagDeployed_Base");

		Raiding.RaidWindows.Insert(new RAPatch_RaidWindow("Fri", "18:00", "23:30"));
		Raiding.RaidWindows.Insert(new RAPatch_RaidWindow("Sat", "10:00", "23:30"));
		Raiding.RaidWindows.Insert(new RAPatch_RaidWindow("Sun", "10:00", "22:00"));
	}

	protected void Validate()
	{
		if (Desk.Radius < 1)
			Desk.Radius = 1;
		if (Desk.MaxPlayers < 0)
			Desk.MaxPlayers = 0;
		if (Crafting.Recipes.Count() > MAX_RECIPES)
			Log("WARNING only the first " + MAX_RECIPES + " recipes are used");
		if (Desk.Modules.Count() > MAX_MODULES)
			Log("WARNING only the first " + MAX_MODULES + " modules are used");
		if (!Crafting.DeskKit)
		{
			RAPatch_Settings defaults = new RAPatch_Settings;
			defaults.FillDefaults();
			Crafting.DeskKit = defaults.Crafting.DeskKit;
		}
	}

	// What a player's game is sent. The settings file itself is only ever on the server. A client needs what its hologram,
	// action texts and recipe list read, and nothing else: raid numbers, decay, no-build zones, admin alerts and test settings stay here.
	// The placement tool
	void RAPatch_PutSpot(RAPatch_VehicleLockSpot spot)
	{
		array<ref RAPatch_VehicleLockSpot> spots = VehicleLocks.Spots;
		for (int i = spots.Count() - 1; i >= 0; i--)
		{
			if (spots[i].VehicleClass == spot.VehicleClass)
				spots.Remove(i);
		}
		spots.Insert(spot);
	}

	void RAPatch_SaveSpot(RAPatch_VehicleLockSpot spot)
	{
		RAPatch_PutSpot(spot);
		string error;
		JsonFileLoader<RAPatch_Settings>.SaveFile(PATH, this, error);
	}

	string ToJsonForClients()
	{
		RAPatch_Settings shared = new RAPatch_Settings;
		shared.Desk = Desk;
		shared.Dismantling = Dismantling;
		shared.Crafting = Crafting;
		shared.Build.Costs = Build.Costs;
		shared.Build.RepairCostMultiplier = Build.RepairCostMultiplier;
		shared.Build.FreePlacement = Build.FreePlacement;
		shared.Build.SlowRotationMultiplier = Build.SlowRotationMultiplier;
		shared.Build.RepairBelowPercent = Build.RepairBelowPercent;
		shared.Build.RepairCostByDamage = Build.RepairCostByDamage;
		shared.Build.UpgradeTools = Build.UpgradeTools;
		shared.Build.HeightLimitMeters = Build.HeightLimitMeters;
		shared.Build.BuildAnywhere = Build.BuildAnywhere;
		shared.Build.BlueprintFromPaper = Build.BlueprintFromPaper;
		shared.Build.BlueprintSeconds = Build.BlueprintSeconds;
		shared.Respawn.BagsEnabled = Respawn.BagsEnabled;
		shared.Respawn.BagPickup = Respawn.BagPickup;
		shared.Respawn.BagsNeedOwnTerritory = Respawn.BagsNeedOwnTerritory;
		shared.Respawn.BagClasses = Respawn.BagClasses;
		shared.VehicleLocks.Enabled = VehicleLocks.Enabled;
		shared.VehicleLocks.LockItem = VehicleLocks.LockItem;
		shared.VehicleLocks.RemoveTool = VehicleLocks.RemoveTool;
		shared.VehicleLocks.RemoveSeconds = VehicleLocks.RemoveSeconds;
		shared.VehicleLocks.BreakTool = VehicleLocks.BreakTool;
		shared.VehicleLocks.BreakSeconds = VehicleLocks.BreakSeconds;
		shared.VehicleLocks.Spots = VehicleLocks.Spots;
		string text;
		string error;
		JsonFileLoader<RAPatch_Settings>.MakeData(shared, text, error, false);
		return text;
	}

	static RAPatch_Settings FromJson(string text)
	{
		RAPatch_Settings settings = new RAPatch_Settings;
		string error;
		if (!JsonFileLoader<RAPatch_Settings>.LoadData(text, settings, error))
		{
			Log("WARNING could not read the server's settings: " + error);
			return null;
		}
		return settings;
	}
}
