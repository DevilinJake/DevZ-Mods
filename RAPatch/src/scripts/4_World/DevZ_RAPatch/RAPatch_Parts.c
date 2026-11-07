// Doors and gates per territory: Desk.MaxDoors and Desk.MaxGates
class RAPatch_Limits
{
	static const int OTHER = 0;
	static const int DOOR = 1;
	static const int GATE = 2;

	static string LastRefusal;	// client: why the part being placed was just refused, for the action's text

	protected static int KindOfClass(string type)
	{
		if (g_Game.IsKindOf(type, "BaseBuilding_SingleDoorBase") || g_Game.IsKindOf(type, "BaseBuilding_DoubleDoorBase"))
			return DOOR;
		if (g_Game.IsKindOf(type, "BaseBuilding_CompoundGateBase") || g_Game.IsKindOf(type, "BaseBuilding_GarageDoorBase"))
			return GATE;
		return OTHER;
	}

	// A blueprint's hologram can be the step before the part it builds, so that's checked too
	static int KindOf(string type)
	{
		int kind = KindOfClass(type);
		if (kind != OTHER)
			return kind;
		string next = BaseBuilding.GetUpgradeType(type);
		if (next == "")
			return OTHER;
		return KindOfClass(next);
	}

	static TerritoryHQ DeskAt(vector position)
	{
		array<TerritoryHQ> desks = TerritoryHQ.All_HQs;
		for (int i = 0; i < desks.Count(); i++)
		{
			TerritoryHQ desk = desks[i];
			if (desk && !desk.IsRuined() && !desk.IsHologram() && vector.Distance(desk.GetPosition(), position) <= desk.GetRadius())
				return desk;
		}
		return null;
	}

	// "" when it can go here
	static string Refusal(string type, vector position)
	{
		int kind = KindOf(type);
		if (kind == OTHER)
			return "";
		RAPatch_DeskSettings rules = RAPatch_Settings.Get().Desk;
		int limit = rules.MaxDoors;
		string name = "Doors";
		if (kind == GATE)
		{
			limit = rules.MaxGates;
			name = "Gates";
		}
		if (limit <= 0)
			return "";
		TerritoryHQ desk = DeskAt(position);
		if (!desk)
			return "";
		int count = desk.RAPatch_DoorCount();
		if (kind == GATE)
			count = desk.RAPatch_GateCount();
		if (count < limit)
			return "";
		return name + " " + count.ToString() + " of " + limit.ToString();
	}
}

// RA's building parts: upkeep, decay, costs, height limit, placement, damage rules, remove window.
modded class BaseBuilding
{
	protected bool m_RAPatch_Destroying;

	// Doors and gates still need a frame: free ones would be doors in thin air
	override PlacementMode GetAllowedPlacementModes()
	{
		PlacementMode modes = super.GetAllowedPlacementModes();
		if (RAPatch_Settings.Get().Build.FreePlacement && !IsInherited(BaseBuilding_OpenableBase))
			return modes | PlacementMode.FREE;
		return modes;
	}

	// RA drops a random amount, half the time. With our percentage on, its own drop is given nothing to make.
	override void AttemptDestructionBehaviour(int oldLevel, int newLevel, string zone)
	{
		if (g_Game.IsServer() && !IsHologram() && !m_RAPatch_Destroying && RAPatch_Settings.Get().Raiding.DestroyedMaterialsPercent >= 0)
		{
			RAPatch_DropMaterials();
			m_RAPatch_Destroying = true;
		}
		super.AttemptDestructionBehaviour(oldLevel, newLevel, zone);
	}

	protected void RAPatch_DropMaterials()
	{
		BaseBuildingItemEntry cost = GetConstructCost();
		if (!cost || cost.Item == "")
			return;
		int left = Math.Round(cost.Quantity * RAPatch_Settings.Get().Raiding.DestroyedMaterialsPercent / 100);
		for (int piece = 0; left > 0 && piece < 40; piece++)
		{
			vector spot = GetPosition() + Vector(Math.RandomFloat(-1, 1), 1.5, Math.RandomFloat(-1, 1));
			ItemBase drop = ItemBase.Cast(g_Game.CreateObjectEx(cost.Item, spot, ECE_FULL));
			if (!drop)
				return;
			int stack = 1;
			if (drop.HasQuantity())
			{
				stack = Math.Min(left, Math.Max(drop.GetQuantityMax(), 1));
				drop.SetQuantity(stack);
			}
			left -= stack;
			drop.SetDirection(vector.RandomDir());
			drop.CreateDynamicPhysics(PhxInteractionLayers.DYNAMICITEM_NOCHAR);
			drop.SetDynamicPhysicsLifeTime(5.0);
		}
	}

	// RA multiplies the full cost by the health a part still has, so the least damaged part costs the most
	override BaseBuildingItemEntry GetCalculatedRepairCost()
	{
		if (!RAPatch_Settings.Get().Build.RepairCostByDamage)
			return super.GetCalculatedRepairCost();
		BaseBuildingItemEntry full = GetRepairCost();
		int quantity = Math.Ceil(full.Quantity * (1 - GetSyncedHealth01()));
		return new BaseBuildingItemEntry(full.Item, Math.Max(quantity, 1));
	}

	bool RAPatch_WantsRepair()
	{
		return GetSyncedHealth01() * 100 < RAPatch_Settings.Get().Build.RepairBelowPercent;
	}

	override float GetCostToUpkeep()
	{
		return RAPatch_Decay.Upkeep(RAPatch_Settings.Get().Decay.PartNailsPerDay);
	}

	override float GetTimeToDecay()
	{
		RAPatch_DecaySettings decay = RAPatch_Settings.Get().Decay;
		switch (GetMaterialLevel())
		{
			case MaterialLevel.FRAME: return RAPatch_Decay.Seconds(decay.FrameHours);
			case MaterialLevel.WOOD: return RAPatch_Decay.Seconds(decay.WoodHours);
			case MaterialLevel.METAL: return RAPatch_Decay.Seconds(decay.MetalHours);
		}
		return RAPatch_Decay.Seconds(decay.ConcreteHours);
	}

	// RA compares "seconds since placed" with a fixed 600 to decide whether a part can still be removed or rotated.
	// That constant can't change, so the placed time is shifted by the difference instead.
	override DateTime GetPlaceTime()
	{
		int shift = RAPatch_Settings.Get().Dismantling.RemoveWindowSeconds - RearmedConstants.ROTATION_COOLDOWN;
		return m_PlaceTime + shift;
	}

	override bool EvaluatePlacementCondition(PlacementMode placement_mode, vector mat[4])
	{
		if (RAPatch_Settings.Get().Build.BuildAnywhere)
			return true;
		return super.EvaluatePlacementCondition(placement_mode, mat);
	}

	override static BaseBuildingItemEntry GetConstructCost(string type)
	{
		return RAPatch_Costs.Apply(type, super.GetConstructCost(type));
	}

	override static BaseBuildingItemEntry GetRepairCost(string type)
	{
		return RAPatch_Costs.ApplyRepair(type, super.GetRepairCost(type));
	}

	// RA's own calls from inside this class go straight to its original static functions, past the two above,
	// so its instance functions have to be redirected too. Calls written as BaseBuilding.GetConstructCost reach ours.
	override BaseBuildingItemEntry GetUpgradeCost()
	{
		return BaseBuilding.GetConstructCost(GetUpgradeType());
	}

	override BaseBuildingItemEntry GetConstructCost()
	{
		if (m_RAPatch_Destroying)
			return new BaseBuildingItemEntry("", 0);
		return BaseBuilding.GetConstructCost(GetType());
	}

	override BaseBuildingItemEntry GetRepairCost()
	{
		return BaseBuilding.GetRepairCost(GetType());
	}

	override static ConstructFailType CheckBuildCondition(notnull PlayerBase player, notnull BaseBuilding base_building, BaseBuildingItemEntry required_materials = null)
	{
		RAPatch_Limits.LastRefusal = "";
		if (base_building.IsHologram())
		{
			RAPatch_Limits.LastRefusal = RAPatch_Limits.Refusal(base_building.GetType(), base_building.GetPosition());
			if (RAPatch_Limits.LastRefusal != "")
				return ConstructFailType.NOBUILDZONE;	// turns the hologram red and blocks it. The action's text says why
		}
		ConstructFailType result = super.CheckBuildCondition(player, base_building, required_materials);
		if (result != ConstructFailType.NONE && result != ConstructFailType.HEIGHT)
			return result;
		if (!RAPatch_Costs.HeightAllowed(base_building.GetPosition()))
			return ConstructFailType.HEIGHT;
		if (result == ConstructFailType.HEIGHT)
		{
			// RA stopped at its own limit, before its last check
			RA_CodeLock code_lock = RA_CodeLock.Cast(base_building.FindAttachmentBySlotName("CodeLock"));
			if (code_lock && code_lock.IsLocked() && !code_lock.IsAuthorized(player.GetIdentity()))
				return ConstructFailType.LOCKED;
		}
		return ConstructFailType.NONE;
	}

	float RAPatch_BlastMultiplier()
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		switch (GetMaterialLevel())
		{
			case MaterialLevel.FRAME: return raid.FrameDamageMultiplier;
			case MaterialLevel.WOOD: return raid.WoodDamageMultiplier;
			case MaterialLevel.METAL: return raid.MetalDamageMultiplier;
			case MaterialLevel.CONCRETE: return raid.ConcreteDamageMultiplier;
		}
		return 1;
	}

	bool RAPatch_CanTakeBlast()
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		foreach (string proof : raid.BlastProofClasses)
		{
			if (IsKindOf(proof))
				return false;
		}
		if (raid.DoorsOnly && !IsInherited(BaseBuilding_OpenableBase))
			return false;
		return true;
	}

	// Melee, bullets, grenades. RA's own charges take health off directly and don't come through here.
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		if (damageType == DamageType.CLOSE_COMBAT && !raid.MeleeDamage)
			return false;
		if (damageType == DamageType.FIRE_ARM && !raid.BulletDamage)
			return false;
		if (damageType == DamageType.EXPLOSION && !raid.OtherExplosiveDamage)
			return false;
		return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}
}

class RAPatch_Costs
{
	protected static RAPatch_CostRow FindRow(string type)
	{
		array<ref RAPatch_CostRow> rows = RAPatch_Settings.Get().Build.Costs;
		if (rows.Count() == 0)
			return null;
		string lowerType = type;
		lowerType.ToLower();
		MaterialLevel level = BaseBuilding.GetMaterialLevel(type);
		string tier = "tier:" + level.GetFormattedName();
		tier.ToLower();
		RAPatch_CostRow tierRow;
		RAPatch_CostRow allRow;
		foreach (RAPatch_CostRow row : rows)
		{
			string name = row.Part;
			name.ToLower();
			if (name == lowerType)
				return row;
			if (name == tier)
				tierRow = row;
			if (name == "tier:all")
				allRow = row;
		}
		if (tierRow)
			return tierRow;
		return allRow;
	}

	static BaseBuildingItemEntry Apply(string type, BaseBuildingItemEntry fromRA)
	{
		RAPatch_CostRow row = FindRow(type);
		if (!row || !fromRA)
			return fromRA;
		if (row.Quantity < 0)
			return null;
		string item = fromRA.Item;
		if (row.Item != "")
			item = row.Item;
		float quantity = fromRA.Quantity;
		if (row.Quantity > 0)
			quantity = row.Quantity;
		quantity = Math.Round(quantity * row.Multiplier);
		// RA reads no entry as "can't be built", so the smallest cost is 1
		return new BaseBuildingItemEntry(item, Math.Max(quantity, 1));
	}

	static BaseBuildingItemEntry ApplyRepair(string type, BaseBuildingItemEntry fromRA)
	{
		if (!fromRA)
			return null;
		float quantity = Math.Round(fromRA.Quantity * RAPatch_Settings.Get().Build.RepairCostMultiplier);
		return new BaseBuildingItemEntry(fromRA.Item, Math.Max(quantity, 1));
	}

	static bool HeightAllowed(vector position)
	{
		return position[1] - g_Game.SurfaceY(position[0], position[2]) <= RAPatch_Settings.Get().Build.HeightLimitMeters;
	}
}

modded class ActionRepairBase
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		BaseBuilding part = BaseBuilding.Cast(target.GetObject());
		if (part && !part.RAPatch_WantsRepair())
			return false;
		return super.ActionCondition(player, target, item);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		BaseBuilding part = BaseBuilding.Cast(action_data.m_Target.GetObject());
		if (part && !part.RAPatch_WantsRepair())
			return;
		super.OnFinishProgressServer(action_data);
	}
}
