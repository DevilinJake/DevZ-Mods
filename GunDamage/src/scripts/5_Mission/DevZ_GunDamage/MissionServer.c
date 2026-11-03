modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		// Creates the settings file on first start, and logs the damage values the game loaded
		DevZ_GunDamageConfig.Get();
		DevZ_GunDamageConfig.LogLoadedValues();
	}
}
