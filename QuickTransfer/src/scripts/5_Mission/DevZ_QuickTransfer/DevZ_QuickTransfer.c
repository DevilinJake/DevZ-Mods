// Hold the Quick Transfer key with the inventory open and move the mouse over items. Each item
// the cursor touches moves, if there's room:
//   from your clothing or backpack           -> the nearest open container in vicinity
//   from a vicinity container or the ground  -> your clothing or backpack
// Runs on the client only. Every move is an ordinary inventory move, so the server checks it
// exactly like a drag and drop.
class DevZ_QuickTransfer
{
	static const string INPUT = "UADevZQuickTransfer";

	protected UAInput m_Input;

	// Items waiting to move, in the order the cursor touched them. The game sends at most one
	// inventory move at a time, so a fast sweep is moved over the next few frames.
	protected ref array<EntityAI> m_Queue = new array<EntityAI>();

	// Items already handled while the key is held, so an item never moves back and forth
	protected ref array<EntityAI> m_Done = new array<EntityAI>();

	void DevZ_QuickTransfer()
	{
		m_Input = GetUApi().GetInputByName(INPUT);
	}

	void Update(VicinityContainer vicinity)
	{
		ItemManager manager = ItemManager.GetInstance();
		if (!m_Input || !manager)
			return;

		EntityAI hovered = manager.GetHoveredItem();
		if (m_Input.LocalValue() > 0 && !manager.IsDragging())
		{
			if (hovered && m_Done.Find(hovered) == -1)
			{
				m_Done.Insert(hovered);
				m_Queue.Insert(hovered);
			}
		}
		else if (!hovered || m_Done.Find(hovered) == -1)
		{
			// Key released and the cursor has left the last item it moved: start fresh. Until
			// then that item stays blocked, so pressing the key again doesn't send it back.
			m_Done.Clear();
		}

		MoveNext(vicinity);
	}

	void Clear()
	{
		m_Queue.Clear();
		m_Done.Clear();
	}

	protected void MoveNext(VicinityContainer vicinity)
	{
		if (m_Queue.Count() == 0 || !ScriptInputUserData.CanStoreInputUserData())
			return;

		EntityAI item = m_Queue[0];
		m_Queue.RemoveOrdered(0);

		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (item && player && player.IsAlive())
			Move(player, item, vicinity);
	}

	protected void Move(PlayerBase player, EntityAI item, VicinityContainer vicinity)
	{
		InventoryLocation src = new InventoryLocation();
		if (!item.GetInventory().GetCurrentInventoryLocation(src) || !item.GetInventory().CanRemoveEntity())
			return;

		InventoryLocation dst;
		int type = src.GetType();
		if (item.GetHierarchyRootPlayer() == player)
		{
			// Only things in your clothing and backpack: never what you wear or hold
			if (type != InventoryLocationType.CARGO)
				return;
			dst = FindNearestOpenContainer(player, item, vicinity);
		}
		else
		{
			if (type != InventoryLocationType.CARGO && type != InventoryLocationType.GROUND)
				return;
			// Hovering a container's header counts as hovering the container: never pick up one
			// you have open, it's the thing you're moving items into
			if (IsOpenInVicinity(item, vicinity))
				return;
			dst = new InventoryLocation();
			if (!player.GetInventory().FindFreeLocationFor(item, FindInventoryLocationType.CARGO, dst))
				return;
		}

		if (dst)
			player.PredictiveTakeToDst(src, dst);
	}

	// The free spot for the item in the nearest container that's open in the vicinity list,
	// skipping containers without room. Bodies are left out: you loot them, you don't fill them.
	protected InventoryLocation FindNearestOpenContainer(PlayerBase player, EntityAI item, VicinityContainer vicinity)
	{
		if (!vicinity)
			return null;

		InventoryLocation best;
		float bestDist = float.MAX;
		vector pos = player.GetPosition();
		foreach (EntityAI container, Container shown : vicinity.m_ShowedItems)
		{
			if (!container || !shown || container == item || !shown.IsOpened())
				continue;
			if (container.IsMan() || container.IsZombie())
				continue;

			float dist = vector.DistanceSq(pos, container.GetPosition());
			if (dist >= bestDist)
				continue;

			InventoryLocation loc = new InventoryLocation();
			if (!container.GetInventory().FindFreeLocationFor(item, FindInventoryLocationType.CARGO, loc))
				continue;

			best = loc;
			bestDist = dist;
		}
		return best;
	}

	protected bool IsOpenInVicinity(EntityAI item, VicinityContainer vicinity)
	{
		if (!vicinity)
			return false;
		Container shown = vicinity.m_ShowedItems.Get(item);
		return shown && shown.IsOpened();
	}
}
