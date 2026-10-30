// Vanilla "Open" on house doors: with a locked Expansion lock it reads "Open locked door" and is
// only there for a player who knows the code (BuildingBase.CanDoorBeOpened). It also works while
// the cursor is on the lock itself rather than the door.
modded class ActionOpenDoors
{
	protected bool m_DevZ_OpenLocked;

	override string GetText()
	{
		if (m_DevZ_OpenLocked)
			return "#STR_EXPANSION_OPEN_LOCKED_DOOR";

		return super.GetText();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		m_DevZ_OpenLocked = false;

		BuildingBase house;
		int doorIndex;
		ExpansionCodeLock codeLock;

		if (DevZ_HouseCodeLock.ResolveLockTarget(target, house, doorIndex, codeLock))
		{
			if (!IsInReach(player, target, UAMaxDistances.DEFAULT))
				return false;

			DoorManipulationParams doorParams = new DoorManipulationParams();
			doorParams.m_DoorIndex = doorIndex;
			doorParams.m_CheckIfLocked = CheckIfDoorIsLocked();
			doorParams.m_Caller = player;
			if (!house.CanDoorBeOpened(doorParams))
				return false;

			m_DevZ_OpenLocked = codeLock.DevZ_HoldsDoor();
			return true;
		}

		if (!super.ActionCondition(player, target, item))
			return false;

		if (DevZ_HouseCodeLock.ResolveDoor(target, house, doorIndex, codeLock) && codeLock)
			m_DevZ_OpenLocked = codeLock.DevZ_HoldsDoor();

		return true;
	}

	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		BuildingBase house;
		int doorIndex;
		ExpansionCodeLock codeLock;
		if (!DevZ_HouseCodeLock.ResolveLockTarget(action_data.m_Target, house, doorIndex, codeLock))
			return;

		DoorManipulationParams doorParams = new DoorManipulationParams();
		doorParams.m_DoorIndex = doorIndex;
		doorParams.m_CheckIfLocked = CheckIfDoorIsLocked();
		doorParams.m_Caller = action_data.m_Player;
		if (house.CanDoorBeOpened(doorParams))
			house.OpenDoor(doorIndex);
	}
}
