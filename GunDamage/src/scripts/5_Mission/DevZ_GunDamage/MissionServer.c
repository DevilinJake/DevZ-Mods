modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		// Creates the settings file on first start, and logs the damage values the game loaded
		DevZ_GunDamageConfig.Get();
		DevZ_GunDamageConfig.LogLoadedValues();
		// Testing only (TestOnStart 1): the hit test, once the world is up
		if (DevZ_GunDamageConfig.Get().TestOnStart)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DevZ_GunDamageSelfTest.Start, 30000, false);
	}
}
