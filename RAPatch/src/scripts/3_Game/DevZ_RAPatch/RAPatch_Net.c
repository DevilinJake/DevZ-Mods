// Clients need some of the settings: the hologram, action texts and recipe list all run on the player's machine.
// Only those go out, held in memory. See ToJsonForClients. The file itself is only ever on the server.
// Sent in pieces, once the player is in the world.
class RAPatch_Net
{
	static const string MOD = "DevZ_RAPatch";
	static const string SETTINGS = "RPC_Settings";
	protected static const int PIECE = 400;
	// Each piece starts with one of these
	protected static const string MORE = "M";
	protected static const string LAST = "L";

	protected static ref RAPatch_Net s_Client;
	protected string m_Received;

	static void StartClient()
	{
		if (!s_Client)
			s_Client = new RAPatch_Net;
	}

	void RAPatch_Net()
	{
		GetRPCManager().AddRPC(MOD, SETTINGS, this, SingleplayerExecutionType.Client);
	}

	static void SendTo(PlayerIdentity identity)
	{
		if (!identity)
			return;
		string text = RAPatch_Settings.Get().ToJsonForClients();
		int length = text.Length();
		int position = 0;
		while (position < length)
		{
			int size = Math.Min(PIECE, length - position);
			string mark = MORE;
			if (position + size >= length)
				mark = LAST;
			GetRPCManager().SendRPC(MOD, SETTINGS, new Param1<string>(mark + text.Substring(position, size)), true, identity);
			position += size;
		}
	}

	void RPC_Settings(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;
		Param1<string> piece;
		if (!ctx.Read(piece))
			return;
		string text = piece.param1;
		m_Received += text.Substring(1, text.Length() - 1);
		if (text.Substring(0, 1) != LAST)
			return;
		RAPatch_Settings settings = RAPatch_Settings.FromJson(m_Received);
		m_Received = "";
		if (settings)
		{
			RAPatch_Settings.Replace(settings);
			RAPatch_Settings.Log("client: got the server's settings");
		}
	}
}

modded class DayZGame
{
	override void DeferredInit()
	{
		super.DeferredInit();
		if (!IsDedicatedServer())
			RAPatch_Net.StartClient();
	}
}
