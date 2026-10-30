modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(DevZ_ActionAttachCodeLockToHouse);
		actions.Insert(DevZ_ActionHouseCodeLock);
		actions.Insert(DevZ_ActionHouseChangeCode);
	}
}
