// Auto-run has to be switched on in two places: in the player's own game and on the server.
// The server runs the same movement code with the keys the client sends it, so if only the
// client overrode its movement, the server would see a standing player and pull them back.
// The client sets its own override (DevZ_AutoRun, 5_Mission) and sends this RPC so the
// server sets the same one; the same message clears it again.

// Picked not to clash with other mods' RPC numbers
const int DEVZ_AUTORUN_RPC = 1514230357;

modded class PlayerBase
{
	// Sets the auto-run override on this side's input controller (1 walk, 2 jog, 3 sprint)
	// or clears it (0). Forward only: the player steers with the camera, as when holding W.
	void DevZ_AutoRun_Apply(int speed)
	{
		HumanInputController hic = GetInputController();
		if (!hic)
			return;

		if (speed > 0)
		{
			hic.OverrideMovementSpeed(HumanInputControllerOverrideType.ENABLED, speed);
			hic.OverrideMovementAngle(HumanInputControllerOverrideType.ENABLED, 0);
		}
		else
		{
			hic.OverrideMovementSpeed(HumanInputControllerOverrideType.DISABLED, 0);
			hic.OverrideMovementAngle(HumanInputControllerOverrideType.DISABLED, 0);
		}
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type == DEVZ_AUTORUN_RPC)
			DevZ_AutoRun_OnRPC(sender, ctx);
	}

	// Server: the player's own client asks for its auto-run state to be mirrored here
	protected void DevZ_AutoRun_OnRPC(PlayerIdentity sender, ParamsReadContext ctx)
	{
		Param1<int> data = new Param1<int>(0);
		if (!ctx.Read(data))
			return;
		if (!g_Game.IsServer())
			return;

		// Only this player's own client may set it
		PlayerIdentity identity = GetIdentity();
		if (!sender || !identity || sender.GetId() != identity.GetId())
			return;

		int speed = data.param1;
		if (speed < 0 || speed > DayZPlayerConstants.MOVEMENTIDX_SPRINT)
			return;

		DevZ_AutoRun_Apply(speed);
	}
}
