// A code lock on any car. The lock lives on the car, not as an attachment: a slot would need every car's config and model
// to have one. The lock item is used up when fitted, and comes back when removed.
//
// Once a code's set there's nothing to lock or unlock: anyone who has entered the code uses the car as normal, and everyone
// else is shut out of it. So a player shot beside their car hasn't left it open.
//
// What players see is an RA code lock drawn on each player's own game, fixed to the car's body. It's only a picture, so a
// lock that's badly placed can't break anything. RA's own keypad opens on it. Placement comes from VehicleLocks.Spots,
// and an admin can set those in-game with the placement tool, with no restart.
class RAPatch_VehicleLocks
{
	static const int NONE = 0;
	static const int PENDING = 1;	// fitted, with no code yet
	static const int OLD_OPEN = 2;	// only in older saves
	static const int LOCKED = 3;

	static ref array<CarScript> Drawn = new array<CarScript>;	// client: cars showing a lock

	static RAPatch_VehicleLockSettings Rules()
	{
		return RAPatch_Settings.Get().VehicleLocks;
	}

	// Aiming at a door or wheel gives you the part, with the car as its parent
	static CarScript CarFrom(ActionTarget target)
	{
		if (!target)
			return null;
		CarScript car = CarScript.Cast(target.GetObject());
		if (!car)
			car = CarScript.Cast(target.GetParent());
		return car;
	}

	// Its own row, or else its nearest relative's, so one colour of a car covers the others
	static RAPatch_VehicleLockSpot SpotFor(string vehicleClass)
	{
		array<ref RAPatch_VehicleLockSpot> spots = Rules().Spots;
		int i;
		for (i = 0; i < spots.Count(); i++)
		{
			if (spots[i].VehicleClass == vehicleClass)
				return spots[i];
		}
		string current = vehicleClass;
		for (int depth = 0; depth < 8; depth++)
		{
			string parent;
			if (!g_Game.ConfigGetBaseName("CfgVehicles " + current, parent) || parent == "")
				return null;
			string lower = parent;
			lower.ToLower();
			if (lower == "carscript")
				return null;
			for (i = 0; i < spots.Count(); i++)
			{
				if (g_Game.IsKindOf(spots[i].VehicleClass, parent))
					return spots[i];
			}
			current = parent;
		}
		return null;
	}

	static bool IsPlacementAdmin(PlayerIdentity identity)
	{
		return identity && Rules().PlacementAdmins.Find(identity.GetPlainId()) != -1;
	}

	static bool IsValidCode(string code)
	{
		if (code.Length() != RA_CodeLock.CODE_LENGTH)
			return false;
		for (int i = 0; i < code.Length(); i++)
		{
			string digit = code.Substring(i, 1);
			if (digit.ToAscii() < 48 || digit.ToAscii() > 57)
				return false;
		}
		return true;
	}
}

modded class CarScript
{
	protected int m_RAPatch_LockState;
	protected int m_RAPatch_LockStamp;	// goes up whenever who's allowed might have changed
	protected string m_RAPatch_LockCode;	// server only: never synced
	protected ref array<string> m_RAPatch_LockUsers = new array<string>;
	protected string m_RAPatch_LockItem;
	protected float m_RAPatch_LockItemHealth;
	protected ref map<string, int> m_RAPatch_WrongCodeAt = new map<string, int>;
	protected bool m_RAPatch_AlarmRunning;
	protected bool m_RAPatch_AlarmHornOn;
	protected int m_RAPatch_AlarmStopAt;

	// Client
	protected bool m_RAPatch_IAmAllowed;
	protected int m_RAPatch_AskedStamp = -1;
	protected RA_CodeLock m_RAPatch_Visual;

	void CarScript()
	{
		RegisterNetSyncVariableInt("m_RAPatch_LockState", RAPatch_VehicleLocks.NONE, RAPatch_VehicleLocks.LOCKED);
		RegisterNetSyncVariableInt("m_RAPatch_LockStamp");
	}

	void ~CarScript()
	{
		if (g_Game && m_RAPatch_AlarmRunning)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(RAPatch_AlarmTick);
		RAPatch_DeleteVisual();
	}

	int RAPatch_LockState()
	{
		return m_RAPatch_LockState;
	}

	bool RAPatch_IsLocked()
	{
		return m_RAPatch_LockState == RAPatch_VehicleLocks.LOCKED && RAPatch_VehicleLocks.Rules().Enabled;
	}

	bool RAPatch_CrewInside()
	{
		for (int i = 0; i < CrewSize(); i++)
		{
			if (CrewMember(i))
				return true;
		}
		return false;
	}

	// Is this player shut out? Actions ask this on both sides: the server by their ID, a player's game by asking the server once.
	bool RAPatch_Denies(PlayerBase player)
	{
		if (!RAPatch_IsLocked())
			return false;
		if (g_Game.IsDedicatedServer())
			return !player.GetIdentity() || RAPatch_IdIsDenied(player.GetIdentity().GetId());
		return !RAPatch_IAmAllowed();
	}

	bool RAPatch_IdIsDenied(string id)
	{
		return RAPatch_IsLocked() && m_RAPatch_LockUsers.Find(id) == -1;
	}

	// ----- server

	protected void RAPatch_LockChanged()
	{
		m_RAPatch_LockStamp++;
		SetSynchDirty();
	}

	// The admin's test: as if this player had never entered the code
	void RAPatch_ForgetUser(string id)
	{
		m_RAPatch_LockUsers.RemoveItem(id);
		RAPatch_LockChanged();
	}

	bool RAPatch_LockUserAllowed(string id)
	{
		return m_RAPatch_LockUsers.Find(id) != -1;
	}

	bool RAPatch_FitLock(ItemBase item)
	{
		if (!g_Game.IsServer() || !item || m_RAPatch_LockState != RAPatch_VehicleLocks.NONE)
			return false;
		m_RAPatch_LockItem = item.GetType();
		m_RAPatch_LockItemHealth = item.GetHealth("", "");
		item.Delete();
		m_RAPatch_LockState = RAPatch_VehicleLocks.PENDING;
		RAPatch_LockChanged();
		return true;
	}

	bool RAPatch_SetCode(string id, string code)
	{
		if (m_RAPatch_LockState != RAPatch_VehicleLocks.PENDING || !RAPatch_VehicleLocks.IsValidCode(code))
			return false;
		m_RAPatch_LockCode = code;
		m_RAPatch_LockUsers.Clear();
		m_RAPatch_LockUsers.Insert(id);
		m_RAPatch_LockState = RAPatch_VehicleLocks.LOCKED;
		RAPatch_LockChanged();
		return true;
	}

	// "" when it's right, and this player's allowed from then on. A short wait after a wrong code, so a code can't simply be run through.
	string RAPatch_TryCode(string id, string code)
	{
		if (m_RAPatch_LockState != RAPatch_VehicleLocks.LOCKED)
			return "This car has no code.";
		int now = g_Game.GetTime();
		int wait = RAPatch_VehicleLocks.Rules().WrongCodeWaitSeconds * 1000;
		if (m_RAPatch_WrongCodeAt.Contains(id) && now - m_RAPatch_WrongCodeAt.Get(id) < wait)
			return "Wait a moment before trying again.";
		if (code != m_RAPatch_LockCode)
		{
			m_RAPatch_WrongCodeAt.Set(id, now);
			return "Wrong code.";
		}
		if (m_RAPatch_LockUsers.Find(id) == -1)
			m_RAPatch_LockUsers.Insert(id);
		return "";
	}

	protected void RAPatch_ClearLock()
	{
		m_RAPatch_LockState = RAPatch_VehicleLocks.NONE;
		m_RAPatch_LockCode = "";
		m_RAPatch_LockItem = "";
		m_RAPatch_LockUsers.Clear();
		RAPatch_LockChanged();
	}

	// Taken off, by someone with the code: the lock item comes back. player null: on the ground beside the car.
	bool RAPatch_RemoveLock(PlayerBase player)
	{
		if (m_RAPatch_LockState == RAPatch_VehicleLocks.NONE)
			return false;
		string type = m_RAPatch_LockItem;
		float health = m_RAPatch_LockItemHealth;
		RAPatch_ClearLock();
		if (type == "")
			return true;
		EntityAI item;
		if (player)
			item = player.GetInventory().CreateInInventory(type);
		if (!item)
			item = EntityAI.Cast(g_Game.CreateObjectEx(type, GetPosition(), ECE_PLACE_ON_SURFACE));
		if (item)
			item.SetHealth("", "", health);
		return true;
	}

	// Breaking in: the lock's lost if it works
	bool RAPatch_BreakLock(float roll)
	{
		if (m_RAPatch_LockState != RAPatch_VehicleLocks.LOCKED)
			return false;
		if (roll >= RAPatch_VehicleLocks.Rules().BreakChance)
			return false;
		RAPatch_ClearLock();
		return true;
	}

	void RAPatch_SoundAlarm(bool on)
	{
		RAPatch_VehicleLockSettings rules = RAPatch_VehicleLocks.Rules();
		if (!g_Game.IsServer() || !rules.AlarmOnBreak)
			return;
		ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY);
		if (m_RAPatch_AlarmRunning)
			queue.Remove(RAPatch_AlarmTick);
		m_RAPatch_AlarmRunning = on;
		m_RAPatch_AlarmHornOn = on;
		if (on)
		{
			SetCarHornState(VehicleHornMode.LONG);
			// Stops by itself if the attempt never reports ending, so the horn can't stick on
			m_RAPatch_AlarmStopAt = g_Game.GetTime() + (rules.BreakSeconds + 5) * 1000;
			queue.CallLater(RAPatch_AlarmTick, 500, true);	// half a second on, half a second off
		}
		else
		{
			SetCarHornState(VehicleHornMode.OFF);
		}
	}

	protected void RAPatch_AlarmTick()
	{
		if (g_Game.GetTime() >= m_RAPatch_AlarmStopAt)
		{
			RAPatch_SoundAlarm(false);
			return;
		}
		m_RAPatch_AlarmHornOn = !m_RAPatch_AlarmHornOn;
		if (m_RAPatch_AlarmHornOn)
			SetCarHornState(VehicleHornMode.LONG);
		else
			SetCarHornState(VehicleHornMode.OFF);
	}

	bool RAPatch_AlarmIsRunning()
	{
		return m_RAPatch_AlarmRunning;
	}

	// ----- client

	bool RAPatch_IAmAllowed()
	{
		if (m_RAPatch_AskedStamp != m_RAPatch_LockStamp)
		{
			m_RAPatch_AskedStamp = m_RAPatch_LockStamp;
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_VehicleLockNet.ASK, null, true, null, this);
		}
		return m_RAPatch_IAmAllowed;
	}

	void RAPatch_SetIAmAllowed(bool allowed)
	{
		m_RAPatch_IAmAllowed = allowed;
		RAPatch_ApplyInventoryHere();
	}

	// Is the car's inventory shut, on this player's game?
	protected bool RAPatch_ShutHere()
	{
		if (g_Game.IsDedicatedServer() || !RAPatch_IsLocked())
			return false;
		return !RAPatch_IAmAllowed();
	}

	protected void RAPatch_ApplyInventoryHere()
	{
		if (g_Game.IsDedicatedServer() || !GetInventory())
			return;
		if (RAPatch_ShutHere())
			GetInventory().LockInventory(HIDE_INV_FROM_SCRIPT);
		else
			GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);
	}

	RA_CodeLock RAPatch_Visual()
	{
		return m_RAPatch_Visual;
	}

	void RAPatch_DeleteVisual()
	{
		if (!m_RAPatch_Visual)
			return;
		RemoveChild(m_RAPatch_Visual);
		if (g_Game)
			g_Game.ObjectDelete(m_RAPatch_Visual);
		m_RAPatch_Visual = null;
		if (RAPatch_VehicleLocks.Drawn)
			RAPatch_VehicleLocks.Drawn.RemoveItem(this);
	}

	// What the lock's placed by right now: the saved spot, or a first guess on the car's left side, for an admin to place properly
	RAPatch_VehicleLockSpot RAPatch_CurrentSpot()
	{
		RAPatch_VehicleLockSpot saved = RAPatch_VehicleLocks.SpotFor(GetType());
		if (saved)
			return saved;
		RAPatch_VehicleLockSpot guess = new RAPatch_VehicleLockSpot;
		guess.VehicleClass = GetType();
		vector minMax[2];
		ClippingInfo(minMax);
		guess.X = minMax[0][0] * 0.9;
		guess.Y = (minMax[0][1] + minMax[1][1]) / 2;
		guess.Z = (minMax[0][2] + minMax[1][2]) / 2;
		guess.Yaw = 90;
		return guess;
	}

	// Draws it where the given spot says. spot null: where it currently belongs.
	void RAPatch_RefreshVisual(RAPatch_VehicleLockSpot spot = null)
	{
		if (g_Game.IsDedicatedServer())
			return;
		bool wanted = m_RAPatch_LockState != RAPatch_VehicleLocks.NONE && RAPatch_VehicleLocks.Rules().Enabled && !IsRuined();
		if (!wanted)
		{
			RAPatch_DeleteVisual();
			return;
		}
		RAPatch_VehicleLockSpot use = spot;
		if (!use)
			use = RAPatch_CurrentSpot();

		if (!m_RAPatch_Visual)
		{
			m_RAPatch_Visual = RA_CodeLock.Cast(g_Game.CreateObjectEx("RA_CodeLock", GetPosition(), ECE_LOCAL));
			if (!m_RAPatch_Visual)
				return;
			RAPatch_VehicleLocks.Drawn.Insert(this);
		}
		else
		{
			RemoveChild(m_RAPatch_Visual);
		}
		vector mat[4];
		Math3D.YawPitchRollMatrix(Vector(use.Yaw, use.Pitch, use.Roll), mat);
		mat[3] = Vector(use.X, use.Y, use.Z);
		m_RAPatch_Visual.SetTransform(mat);
		AddChild(m_RAPatch_Visual, -1, false);
		m_RAPatch_Visual.Update();
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		RAPatch_RefreshVisual();
		RAPatch_ApplyInventoryHere();
	}

	// ----- what a lock stops

	override bool IsInventoryVisible()
	{
		if (RAPatch_ShutHere())
			return false;
		return super.IsInventoryVisible();
	}

	override bool CanDisplayAttachmentSlot(int slot_id)
	{
		if (RAPatch_ShutHere())
			return false;
		return super.CanDisplayAttachmentSlot(slot_id);
	}

	override bool CanDisplayAttachmentCategory(string category_name)
	{
		if (RAPatch_ShutHere())
			return false;
		return super.CanDisplayAttachmentCategory(category_name);
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(RAPatch_ActionCarFitLock);
		AddAction(RAPatch_ActionCarSetCode);
		AddAction(RAPatch_ActionCarEnterCode);
		AddAction(RAPatch_ActionCarRemoveLock);
		AddAction(RAPatch_ActionCarBreakLock);
		AddAction(RAPatch_ActionCarPlaceLock);
		AddAction(RAPatch_ActionCarForgetCode);
	}

	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return;
		ctx.Write(m_RAPatch_LockState);
		ctx.Write(m_RAPatch_LockCode);
		ctx.Write(m_RAPatch_LockItem);
		ctx.Write(m_RAPatch_LockItemHealth);
		ctx.Write(m_RAPatch_LockUsers.Count());
		for (int i = 0; i < m_RAPatch_LockUsers.Count(); i++)
			ctx.Write(m_RAPatch_LockUsers[i]);
	}

	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage))
			return false;
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return true;
		int count;
		if (!ctx.Read(m_RAPatch_LockState) || !ctx.Read(m_RAPatch_LockCode) || !ctx.Read(m_RAPatch_LockItem) || !ctx.Read(m_RAPatch_LockItemHealth) || !ctx.Read(count))
			return false;
		for (int i = 0; i < count; i++)
		{
			string id;
			if (!ctx.Read(id))
				return false;
			m_RAPatch_LockUsers.Insert(id);
		}
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		if (m_RAPatch_LockState == RAPatch_VehicleLocks.OLD_OPEN)
			m_RAPatch_LockState = RAPatch_VehicleLocks.LOCKED;
		RAPatch_LockChanged();
	}
}

// ----- actions. The car offers them, so they work whatever's in hand, and the condition checks it.

class RAPatch_ActionCarContinuousCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		RAPatch_ActionCarContinuous action = RAPatch_ActionCarContinuous.Cast(m_ActionData.m_Action);
		float seconds = 5;
		if (action)
			seconds = action.RAPatch_Seconds();
		m_ActionData.m_ActionComponent = new CAContinuousTime(seconds);
	}
}

class RAPatch_ActionCarContinuous : ActionContinuousBase
{
	void RAPatch_ActionCarContinuous()
	{
		m_CallbackClass = RAPatch_ActionCarContinuousCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
	}

	float RAPatch_Seconds()
	{
		return 5;
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool HasTarget()
	{
		return true;
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor;
		m_ConditionItem = new CCINonRuined;
	}
}

class RAPatch_ActionCarBreakLock : RAPatch_ActionCarContinuous
{
	void RAPatch_ActionCarBreakLock()
	{
		m_Text = "Force code lock";
	}

	override float RAPatch_Seconds()
	{
		return RAPatch_VehicleLocks.Rules().BreakSeconds;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		RAPatch_VehicleLockSettings rules = RAPatch_VehicleLocks.Rules();
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (!rules.Enabled || !car || !item || rules.BreakTool == "" || !item.IsKindOf(rules.BreakTool))
			return false;
		return car.RAPatch_Denies(player);
	}

	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (car)
			car.RAPatch_SoundAlarm(true);
	}

	override void OnEndServer(ActionData action_data)
	{
		super.OnEndServer(action_data);
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (car)
			car.RAPatch_SoundAlarm(false);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (!car)
			return;
		string message = "It held.";
		if (car.RAPatch_BreakLock(Math.RandomFloat01()))
			message = "The lock's broken.";
		ItemBase tool = action_data.m_MainItem;
		if (tool)
			tool.DecreaseHealth("", "", tool.GetMaxHealth("", "") * RAPatch_VehicleLocks.Rules().BreakToolWearPercent / 100);
		NotificationSystem.SendNotificationToPlayerExtended(action_data.m_Player, 5, "Code lock", message);
	}
}

class RAPatch_ActionCarRemoveLock : RAPatch_ActionCarContinuous
{
	void RAPatch_ActionCarRemoveLock()
	{
		m_Text = "Remove code lock";
	}

	override float RAPatch_Seconds()
	{
		return RAPatch_VehicleLocks.Rules().RemoveSeconds;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		RAPatch_VehicleLockSettings rules = RAPatch_VehicleLocks.Rules();
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (!rules.Enabled || !car || !item || !item.IsKindOf(rules.RemoveTool))
			return false;
		return car.RAPatch_LockState() != RAPatch_VehicleLocks.NONE && !car.RAPatch_Denies(player);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (car)
			car.RAPatch_RemoveLock(action_data.m_Player);
	}
}

class RAPatch_ActionCarInteract : ActionInteractBase
{
	void RAPatch_ActionCarInteract()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	// The keypad, on the lock this player's game is drawing
	protected void RAPatch_OpenKeypad(CarScript car, bool setting)
	{
		#ifndef SERVER
		RA_CodeLock lock = car.RAPatch_Visual();
		if (!lock)
			return;
		RAPatch_VehicleLockNet.KeypadCar = car;
		RAPatch_VehicleLockNet.KeypadSetting = setting;
		lock.OpenMenu(ScriptCaller.Create(RAPatch_VehicleLockNet.Get().OnKeypadCode));
		#endif
	}
}

class RAPatch_ActionCarFitLock : RAPatch_ActionCarInteract
{
	void RAPatch_ActionCarFitLock()
	{
		m_Text = "Fit code lock";
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ATTACHITEM;	// the animation for attaching a lock to a door
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		// An interact action isn't given the item in hand, so it looks for itself
		RAPatch_VehicleLockSettings rules = RAPatch_VehicleLocks.Rules();
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		ItemBase held = player.GetItemInHands();
		if (!rules.Enabled || !car || !held || held.IsRuined() || car.IsRuined() || !held.IsKindOf(rules.LockItem))
			return false;
		return car.RAPatch_LockState() == RAPatch_VehicleLocks.NONE && !car.RAPatch_CrewInside();
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (car)
			car.RAPatch_FitLock(action_data.m_Player.GetItemInHands());
	}
}

class RAPatch_ActionCarSetCode : RAPatch_ActionCarInteract
{
	void RAPatch_ActionCarSetCode()
	{
		m_Text = "Set code";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		return RAPatch_VehicleLocks.Rules().Enabled && car && !item && car.RAPatch_LockState() == RAPatch_VehicleLocks.PENDING;
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (car)
			RAPatch_OpenKeypad(car, true);
	}
}

class RAPatch_ActionCarEnterCode : RAPatch_ActionCarInteract
{
	void RAPatch_ActionCarEnterCode()
	{
		m_Text = "Enter code";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		return car && !item && car.RAPatch_Denies(player);
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		if (car)
			RAPatch_OpenKeypad(car, false);
	}
}

class RAPatch_ActionCarPlaceLock : RAPatch_ActionCarInteract
{
	void RAPatch_ActionCarPlaceLock()
	{
		m_Text = "[ADMIN] Place code lock";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (!RAPatch_VehicleLocks.Rules().Enabled || !car || item)
			return false;
		if (g_Game.IsDedicatedServer())
			return RAPatch_VehicleLocks.IsPlacementAdmin(player.GetIdentity());
		return RAPatch_VehicleLockNet.IAmPlacementAdmin && car.RAPatch_Visual() != null;
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		RAPatch_VehicleLockNet.PlacerRequest = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
	}
}

class RAPatch_ActionCarForgetCode : RAPatch_ActionCarInteract
{
	void RAPatch_ActionCarForgetCode()
	{
		m_Text = "[ADMIN] Forget my code";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (!car || item || !car.RAPatch_IsLocked() || car.RAPatch_Denies(player))
			return false;
		if (g_Game.IsDedicatedServer())
			return RAPatch_VehicleLocks.IsPlacementAdmin(player.GetIdentity());
		return RAPatch_VehicleLockNet.IAmPlacementAdmin;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);
		CarScript car = RAPatch_VehicleLocks.CarFrom(action_data.m_Target);
		PlayerIdentity identity = action_data.m_Player.GetIdentity();
		if (car && identity && RAPatch_VehicleLocks.IsPlacementAdmin(identity))
			car.RAPatch_ForgetUser(identity.GetId());
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(RAPatch_ActionCarFitLock);
		actions.Insert(RAPatch_ActionCarSetCode);
		actions.Insert(RAPatch_ActionCarEnterCode);
		actions.Insert(RAPatch_ActionCarRemoveLock);
		actions.Insert(RAPatch_ActionCarBreakLock);
		actions.Insert(RAPatch_ActionCarPlaceLock);
		actions.Insert(RAPatch_ActionCarForgetCode);
	}
}

// ----- what a locked car refuses anyone without the code

modded class ActionGetInTransport
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (car && car.RAPatch_Denies(player))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

modded class ActionCarDoors
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (car && car.RAPatch_Denies(player))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

modded class ActionCarDoorsOutside
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (car && car.RAPatch_Denies(player))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

modded class ActionAnimateCarSelection
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car = RAPatch_VehicleLocks.CarFrom(target);
		if (car && car.RAPatch_Denies(player))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

modded class ActionDetach
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		CarScript car;
		if (target)
			car = CarScript.Cast(target.GetParent());
		if (car && car.RAPatch_Denies(player))
			return false;
		return super.ActionCondition(player, target, item);
	}
}

// ----- messages. The server checks every request: close enough, and allowed.

class RAPatch_VehicleLockNet
{
	static const string ASK = "RPC_LockAsk";
	static const string ANSWER = "RPC_LockAnswer";
	static const string SET_CODE = "RPC_LockSetCode";
	static const string TRY_CODE = "RPC_LockTryCode";
	static const string AM_I_ADMIN = "RPC_LockAmIAdmin";
	static const string IS_ADMIN = "RPC_LockIsAdmin";
	static const string SAVE_SPOT = "RPC_LockSaveSpot";
	static const string SPOT = "RPC_LockSpot";
	protected static const float MAX_DISTANCE = 12;

	protected static ref RAPatch_VehicleLockNet s_Instance;

	// Client
	static bool IAmPlacementAdmin;
	static CarScript KeypadCar;
	static bool KeypadSetting;
	static CarScript PlacerRequest;

	static RAPatch_VehicleLockNet Get()
	{
		if (!s_Instance)
			s_Instance = new RAPatch_VehicleLockNet;
		return s_Instance;
	}

	void RAPatch_VehicleLockNet()
	{
		GetRPCManager().AddRPC(RAPatch_Net.MOD, ASK, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, AM_I_ADMIN, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, SET_CODE, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, TRY_CODE, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, SAVE_SPOT, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, ANSWER, this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, IS_ADMIN, this, SingleplayerExecutionType.Client);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, SPOT, this, SingleplayerExecutionType.Client);
		#ifndef SERVER
		RAPatch_Settings.OnReplaced.Insert(RedrawAll);
		#endif
	}

	void ~RAPatch_VehicleLockNet()
	{
		#ifndef SERVER
		if (RAPatch_Settings.OnReplaced)
			RAPatch_Settings.OnReplaced.Remove(RedrawAll);
		#endif
	}

	// The server's settings can arrive after a car is drawn, and they hold the saved spots
	void RedrawAll()
	{
		array<CarScript> cars = RAPatch_VehicleLocks.Drawn;
		for (int i = cars.Count() - 1; i >= 0; i--)
		{
			if (cars[i])
				cars[i].RAPatch_RefreshVisual();
		}
	}

	static void AskIfAdmin()
	{
		GetRPCManager().SendRPC(RAPatch_Net.MOD, AM_I_ADMIN, null, true);
	}

	void RPC_LockAmIAdmin(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type == CallType.Server && RAPatch_VehicleLocks.IsPlacementAdmin(sender))
			GetRPCManager().SendRPC(RAPatch_Net.MOD, IS_ADMIN, new Param1<bool>(true), true, sender);
	}

	protected CarScript AllowedCar(PlayerIdentity sender, Object target)
	{
		CarScript car = CarScript.Cast(target);
		if (!car || !sender)
			return null;
		PlayerBase player = PlayerBase.Cast(sender.GetPlayer());
		if (!player || vector.Distance(player.GetPosition(), car.GetPosition()) > MAX_DISTANCE)
			return null;
		return car;
	}

	protected void Answer(PlayerIdentity to, CarScript car)
	{
		GetRPCManager().SendRPC(RAPatch_Net.MOD, ANSWER, new Param1<bool>(car.RAPatch_LockUserAllowed(to.GetId())), true, to, car);
	}

	protected void Tell(PlayerIdentity to, string message)
	{
		NotificationSystem.SendNotificationToPlayerIdentityExtended(to, 5, "Code lock", message);
	}

	// Only ever says whether the one asking is allowed, so it answers at any distance: a game asks as soon as it sees the car
	void RPC_LockAsk(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server || !sender)
			return;
		CarScript car = CarScript.Cast(target);
		if (car)
			Answer(sender, car);
	}

	void RPC_LockSetCode(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		CarScript car = AllowedCar(sender, target);
		if (car && car.RAPatch_SetCode(sender.GetId(), data.param1))
		{
			Answer(sender, car);
			Tell(sender, "Code set. Only people with the code can use this car.");
		}
	}

	void RPC_LockTryCode(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		CarScript car = AllowedCar(sender, target);
		if (!car)
			return;
		string refusal = car.RAPatch_TryCode(sender.GetId(), data.param1);
		if (refusal == "")
		{
			Answer(sender, car);
			Tell(sender, "Code accepted. You can use this car from now on.");
		}
		else
		{
			Tell(sender, refusal);
		}
	}

	void RPC_LockSaveSpot(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		Param1<string> data;
		if (!ctx.Read(data) || !RAPatch_VehicleLocks.IsPlacementAdmin(sender))
			return;
		RAPatch_VehicleLockSpot spot = RAPatch_VehicleLockSpot.FromJson(data.param1);
		if (!spot)
			return;
		RAPatch_Settings.Get().RAPatch_SaveSpot(spot);
		RAPatch_Settings.Log("saved the code lock's spot on " + spot.VehicleClass);
		// Everyone online sees it straight away
		GetRPCManager().SendRPC(RAPatch_Net.MOD, SPOT, new Param1<string>(data.param1), true, null);
		Tell(sender, "Saved for " + spot.VehicleClass + ".");
	}

	void RPC_LockAnswer(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;
		Param1<bool> data;
		if (!ctx.Read(data))
			return;
		CarScript car = CarScript.Cast(target);
		if (car)
			car.RAPatch_SetIAmAllowed(data.param1);
	}

	void RPC_LockIsAdmin(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type == CallType.Client)
			IAmPlacementAdmin = true;
	}

	void RPC_LockSpot(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		RAPatch_VehicleLockSpot spot = RAPatch_VehicleLockSpot.FromJson(data.param1);
		if (!spot)
			return;
		RAPatch_Settings.Get().RAPatch_PutSpot(spot);
		RedrawAll();
	}

	// RA's keypad hands back the code when four digits are in
	void OnKeypadCode(RA_CodeLock code_lock, string code)
	{
		if (code_lock)
			code_lock.CloseMenu();
		if (!KeypadCar)
			return;
		string rpc = TRY_CODE;
		if (KeypadSetting)
			rpc = SET_CODE;
		GetRPCManager().SendRPC(RAPatch_Net.MOD, rpc, new Param1<string>(code), true, null, KeypadCar);
		KeypadCar = null;
	}
}
