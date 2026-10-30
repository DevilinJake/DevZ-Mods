// Carry the kit (a heavy item) with the wooden crate hold
modded class JMAnimRegister
{
	override void OnRegisterHeavy(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
	{
		super.OnRegisterHeavy(pType, pBehavior);
		pType.AddItemInHandsProfileIK("BarrelStorage_Kit", "dz/anims/workspaces/player/player_main/player_main_heavy.asi", pBehavior, "dz/anims/anm/player/ik/heavy/wooden_crate.anm");
	}
};
