// Press the Auto Run key and the player keeps moving forward at the speed they had: walking,
// jogging or sprinting (jogging if they stood still). Any movement key, or the key again,
// stops it. Jumping, vaulting and changing stance don't.
//
// Runs on the client only. The movement override itself is set in PlayerBase
// (4_World\DevZ_AutoRun\PlayerBase.c), on this side and, by RPC, on the server.
class DevZ_AutoRun
{
	static const string INPUT = "UADevZAutoRun";

	protected UAInput m_Toggle;
	protected UAInput m_Forward;
	protected UAInput m_Back;
	protected UAInput m_Left;
	protected UAInput m_Right;

	protected PlayerBase m_Player;		// the character auto-running: a new one (after death) starts off
	protected int m_Speed;				// what was asked for: 1 walk, 2 jog, 3 sprint; 0 = off
	protected int m_Applied;			// what is in force right now (sprint drops to jog without stamina)
	protected ref HumanMovementState m_State = new HumanMovementState();

	// Called every frame by MissionGameplay. While auto-run is off, this is one key check.
	void Update()
	{
		if (!m_Toggle)
			Init();

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (m_Speed > 0 && player != m_Player)
			Reset();
		if (!player)
			return;

		UIScriptedMenu menu = g_Game.GetUIManager().GetMenu();
		bool keysFree = !IsTyping() && (!menu || KeepsRunningIn(menu));

		if (keysFree && m_Toggle.LocalPress())
		{
			if (m_Speed > 0)
				Stop();
			else
				Start(player);
			return;
		}

		if (m_Speed == 0)
			return;

		if (!CanRun(player))
		{
			Stop();
			return;
		}
		if (menu && !KeepsRunningIn(menu))
		{
			Stop();
			return;
		}
		if (keysFree && (m_Forward.LocalPress() || m_Back.LocalPress() || m_Left.LocalPress() || m_Right.LocalPress()))
		{
			Stop();
			return;
		}

		// Sprint ends with the stamina and comes back with it, the way holding Shift does
		if (m_Speed == DayZPlayerConstants.MOVEMENTIDX_SPRINT)
		{
			int speed = DayZPlayerConstants.MOVEMENTIDX_SPRINT;
			HumanInputController hic = player.GetInputController();
			if (hic && hic.LimitsIsSprintDisabled())
				speed = DayZPlayerConstants.MOVEMENTIDX_RUN;
			if (speed != m_Applied)
				Apply(speed);
		}
	}

	protected void Init()
	{
		UAInputAPI api = GetUApi();
		m_Toggle = api.GetInputByName(INPUT);
		m_Forward = api.GetInputByName("UAMoveForward");
		m_Back = api.GetInputByName("UAMoveBack");
		m_Left = api.GetInputByName("UAMoveLeft");
		m_Right = api.GetInputByName("UAMoveRight");
	}

	protected void Start(PlayerBase player)
	{
		if (!CanRun(player))
			return;

		// Keep the speed the player has: walk, jog or sprint
		player.GetMovementState(m_State);
		int speed = m_State.m_iMovement;
		if (speed >= DayZPlayerConstants.MOVEMENTIDX_SPRINT)
		{
			speed = DayZPlayerConstants.MOVEMENTIDX_SPRINT;
		}
		else if (speed < DayZPlayerConstants.MOVEMENTIDX_WALK)
		{
			// Standing still: jog, or walk if walking is toggled on
			speed = DayZPlayerConstants.MOVEMENTIDX_RUN;
			HumanInputController hic = player.GetInputController();
			if (hic && hic.IsWalkToggled())
				speed = DayZPlayerConstants.MOVEMENTIDX_WALK;
		}

		m_Player = player;
		m_Speed = speed;
		Apply(speed);
	}

	protected void Stop()
	{
		if (m_Player)
			Apply(0);
		Reset();
	}

	protected void Reset()
	{
		m_Player = null;
		m_Speed = 0;
		m_Applied = 0;
	}

	// Sets the override on this side and tells the server to set the same one (0 clears it)
	protected void Apply(int speed)
	{
		m_Applied = speed;
		m_Player.DevZ_AutoRun_Apply(speed);
		if (g_Game.IsMultiplayer())
			g_Game.RPCSingleParam(m_Player, DEVZ_AUTORUN_RPC, new Param1<int>(speed), true);
	}

	// Dead, knocked out, tied up or in a vehicle: no auto-run
	protected bool CanRun(PlayerBase player)
	{
		return player.IsAlive() && !player.IsUnconscious() && !player.IsRestrained() && !player.IsInVehicle();
	}

	// Screens the game lets you walk through with the movement keys (or type in): auto-run
	// keeps going there too. Any other menu (Esc, a trader, a mod's screens) stops it.
	protected bool KeepsRunningIn(UIScriptedMenu menu)
	{
		int id = menu.GetID();
		return id == MENU_INVENTORY || id == MENU_MAP || id == MENU_NOTE || id == MENU_GESTURES || id == MENU_RADIAL_QUICKBAR || id == MENU_CHAT_INPUT;
	}

	// True while the keyboard is typing into a text box (chat, a note), so letters aren't keys
	protected bool IsTyping()
	{
		Widget focus = GetFocus();
		if (!focus || !focus.IsVisible())
			return false;
		return focus.IsInherited(EditBoxWidget) || focus.IsInherited(MultilineEditBoxWidget);
	}
}
