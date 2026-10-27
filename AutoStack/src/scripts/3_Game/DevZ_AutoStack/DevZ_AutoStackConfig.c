// Settings, saved to $profile:DevZ_AutoStack\Config.json on first start. Edit there and restart.
class DevZ_AutoStackConfig
{
	static const string FOLDER = "$profile:DevZ_AutoStack\\";
	static const string FILE = FOLDER + "Config.json";

	int Enabled = 1;

	// Merge ammo into other stacks of the same ammo anywhere in the player's clothing and backpack
	int StackInPlayerInventory = 1;

	// Merge ammo put into a container (crate, tent, car trunk, backpack on the ground) with stacks
	// of the same ammo already in that container
	int StackInContainers = 1;

	// Wait this long after an item moves before merging, so the move finishes first
	int DelayMs = 300;

	protected static ref DevZ_AutoStackConfig s_Instance;

	static DevZ_AutoStackConfig Get()
	{
		if (!s_Instance)
		{
			s_Instance = new DevZ_AutoStackConfig();
			if (FileExist(FILE))
			{
				JsonFileLoader<DevZ_AutoStackConfig>.JsonLoadFile(FILE, s_Instance);
			}
			else
			{
				MakeDirectory(FOLDER);
				JsonFileLoader<DevZ_AutoStackConfig>.JsonSaveFile(FILE, s_Instance);
			}
		}
		return s_Instance;
	}
}
