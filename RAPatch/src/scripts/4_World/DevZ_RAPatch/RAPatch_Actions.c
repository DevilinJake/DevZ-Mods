// Who can do what, and how long it takes. RA's own conditions and timers stay in place: the patch adds its rule
// in front of them, on the client and on the server.
class RAPatch_Tools
{
	static RAPatch_ToolRow Find(ItemBase tool)
	{
		if (!tool)
			return null;
		foreach (RAPatch_ToolRow row : RAPatch_Settings.Get().Build.UpgradeTools)
		{
			if (tool.IsKindOf(row.Tool))
				return row;
		}
		return null;
	}
}

modded class ActionUpgradeBaseCB
{
	override void CreateActionComponent()
	{
		super.CreateActionComponent();
		RAPatch_ToolRow row = RAPatch_Tools.Find(ItemBase.Cast(m_ActionData.m_MainItem));
		if (row)
			m_ActionData.m_ActionComponent = new CAContinuousTime(row.Seconds);
	}
}

modded class ActionUpgradeBase
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_Tools.Find(item))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

modded class ActionRepairBase
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_Tools.Find(item))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

// RA gives Hammer and Hatchet its building actions. Other tools have to be named in code; listing them in
// Building.UpgradeTools is then what switches them on.
modded class SledgeHammer
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionUpgradeBase);
		AddAction(ActionRepairBase);
	}
}

modded class ActionDismantleBase
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_Settings.Get().Dismantling.ToolRemoveEnabled)
			return false;
		return super.ActionCondition(player, target, item);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		if (!RAPatch_Settings.Get().Dismantling.ToolRemoveEnabled)
			return;
		super.OnFinishProgressServer(action_data);
	}
}

// RA's own check on this action only asks whether the desk has a lock, so anyone can pack up an unlocked desk.
modded class ActionDismantleIntoBoxCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(RAPatch_Settings.Get().Dismantling.DeskPackUpSeconds);
	}
}

modded class ActionDismantleIntoBox
{
	bool RAPatch_MayPackUp(PlayerBase player, Object target)
	{
		TerritoryHQ hq = TerritoryHQ.Cast(target);
		if (!hq)
			return true;
		// Its storage would go with it
		if (hq.RAPatch_CargoCount() > 0)
			return false;
		int mode = RAPatch_Settings.Get().Dismantling.DeskPackUp;
		if (mode == 0)
			return true;
		if (mode == 1)
			return hq.IsAuthorized(player);
		return false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_MayPackUp(player, target.GetObject()))
			return false;
		return super.ActionCondition(player, target, item);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		if (!RAPatch_MayPackUp(action_data.m_Player, action_data.m_Target.GetObject()))
			return;
		TerritoryHQ hq = TerritoryHQ.Cast(action_data.m_Target.GetObject());
		if (hq && !hq.FindCodeLock())
			hq.RAPatch_DropModules();
		super.OnFinishProgressServer(action_data);
	}
}

// Vanilla parts only: a watchtower. RA's parts have their own Remove, above. This is where RA's two BASE_DISMANTLE numbers end up.
modded class ActionDismantlePartCB
{
	override float SetCallbackDuration(ItemBase item)
	{
		RAPatch_DismantleSettings rules = RAPatch_Settings.Get().Dismantling;
		if (super.SetCallbackDuration(item) == RearmedConstants.BASE_DISMANTLE_NO_PERMISSION)
			return rules.VanillaSecondsOutsider;
		return rules.VanillaSecondsAuthorized;
	}
}

modded class ActionDismantlePart
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_Settings.Get().Dismantling.VanillaOutsidersAllowed && target.GetObject())
		{
			if (!TerritoryHQ.HasPermissionsAtPosition(player, target.GetObject().GetPosition()))
				return false;
		}
		return super.ActionCondition(player, target, item);
	}
}

modded class ActionTurnIntoBlueprintCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(RAPatch_Settings.Get().Build.BlueprintSeconds);
	}
}

modded class ActionTurnIntoBlueprint
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_Settings.Get().Build.BlueprintFromPaper)
			return false;
		return super.ActionCondition(player, target, item);
	}
}

// The client decides where a part goes, and RA's server only checks permission there. This makes the server check the rest too.
modded class ActionPlaceHologram
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!super.ActionCondition(player, target, item))
			return false;
		if (RAPatch_Limits.LastRefusal != "")
			m_Text = "Cannot Build (" + RAPatch_Limits.LastRefusal + ")";
		return true;
	}

	override void OnStartServer(ActionData action_data)
	{
		PlaceHologramActionData placing = PlaceHologramActionData.Cast(action_data);
		if (placing)
		{
			string refusal = RAPatch_Limits.Refusal(placing.Type, placing.Position);
			if (refusal != "")
			{
				NotificationSystem.SendNotificationToPlayerExtended(action_data.m_Player, 5, "Can't build here", "This territory is at its limit. " + refusal + ".");
				return;
			}
		}
		if (RAPatch_Settings.Get().Build.ServerChecksSpot)
		{
			PlaceHologramActionData data = PlaceHologramActionData.Cast(action_data);
			if (data && (!RAPatch_Costs.HeightAllowed(data.Position) || !NoBuildZone.HasPermissionsAtPosition(data.Position)))
			{
				RAPatch_Settings.Log("refused a part at " + data.Position + ": too high or in a no-build zone");
				return;
			}
		}
		super.OnStartServer(action_data);
	}
}

// Who can deploy inside a territory: Desk.DeployRule. RA's charges have their own class and aren't touched.
modded class ItemBase
{
	override bool CanPlaceInEnemyTerritory()
	{
		return RAPatch_Settings.Get().Desk.DeployRule == 1;
	}
}

// Building parts have their own rules: "anyone can deploy" doesn't mean anyone can build
modded class BaseBuilding
{
	override bool CanPlaceInEnemyTerritory()
	{
		return false;
	}
}

// Traps and mines: only rule 0 keeps them out
modded class BearTrap
{
	override bool CanPlaceInEnemyTerritory()
	{
		return RAPatch_Settings.Get().Desk.DeployRule != 0;
	}
}

modded class ClaymoreMine
{
	override bool CanPlaceInEnemyTerritory()
	{
		return RAPatch_Settings.Get().Desk.DeployRule != 0;
	}
}

modded class LandMineTrap
{
	override bool CanPlaceInEnemyTerritory()
	{
		return RAPatch_Settings.Get().Desk.DeployRule != 0;
	}
}

modded class TripwireTrap
{
	override bool CanPlaceInEnemyTerritory()
	{
		return RAPatch_Settings.Get().Desk.DeployRule != 0;
	}
}

modded class ActionDeployObject
{
	// RA checks where the player's standing, so someone just outside a territory could place inside it
	override bool ActionConditionContinue(ActionData action_data)
	{
		if (g_Game.IsDedicatedServer() && RAPatch_Settings.Get().Desk.DeployChecksPlacedSpot)
		{
			Hologram hologram = action_data.m_Player.GetHologramServer();
			if (hologram && hologram.GetProjectionEntity() && !action_data.m_MainItem.CanPlaceInEnemyTerritory())
			{
				if (!TerritoryHQ.HasPermissionsAtPosition(action_data.m_Player, hologram.GetProjectionEntity().GetPosition()))
					return false;
			}
		}
		return super.ActionConditionContinue(action_data);
	}
}
