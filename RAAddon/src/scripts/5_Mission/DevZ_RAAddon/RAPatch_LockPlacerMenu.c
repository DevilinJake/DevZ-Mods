// The admin's placement tool: nudge the code lock on the car in front of you, see it move, and save it for every car of that
// class. It saves to Settings.json on the server, and everyone online sees it at once, with no restart.
class RAPatch_LockPlacerMenu : UIScriptedMenu
{
	protected static const float MOVE_STEPS[3] = {0.005, 0.02, 0.1};
	protected static const float TURN_STEPS[3] = {1, 5, 15};

	protected CarScript m_Car;
	protected ref RAPatch_VehicleLockSpot m_Spot;
	protected int m_StepIndex = 1;

	protected TextWidget m_Title;
	protected TextWidget m_Readout;
	protected ButtonWidget m_Step;
	protected TextWidget m_StepLabel;
	protected ButtonWidget m_Save;
	protected ButtonWidget m_Close;

	void SetCar(CarScript car)
	{
		m_Car = car;
		m_Spot = new RAPatch_VehicleLockSpot;
		RAPatch_VehicleLockSpot current = car.RAPatch_CurrentSpot();
		m_Spot.VehicleClass = car.GetType();
		m_Spot.X = current.X;
		m_Spot.Y = current.Y;
		m_Spot.Z = current.Z;
		m_Spot.Yaw = current.Yaw;
		m_Spot.Pitch = current.Pitch;
		m_Spot.Roll = current.Roll;
		m_Title.SetText(car.GetType());
		ShowSpot();
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("DevZ_RAAddon/gui/lock_placer.layout");
		m_Title = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleText"));
		m_Readout = TextWidget.Cast(layoutRoot.FindAnyWidget("Readout"));
		m_Step = ButtonWidget.Cast(layoutRoot.FindAnyWidget("StepButton"));
		m_StepLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("StepButtonLabel"));
		m_Save = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SaveButton"));
		m_Close = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));
		return layoutRoot;
	}

	override bool UseMouse()
	{
		return true;
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override void OnShow()
	{
		super.OnShow();
		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
	}

	override void OnHide()
	{
		super.OnHide();
		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor(false);
		GetGame().GetMission().PlayerControlEnable(true);
		// Back to what's saved, if this wasn't
		if (m_Car)
			m_Car.RAPatch_RefreshVisual();
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (!m_Car || RAPatch_MenuKeys.EscPressed())
			Close();
	}

	protected void ShowSpot()
	{
		if (!m_Car || !m_Spot)
			return;
		m_Car.RAPatch_RefreshVisual(m_Spot);
		string readout = "X " + m_Spot.X.ToString() + "   Y " + m_Spot.Y.ToString() + "   Z " + m_Spot.Z.ToString();
		readout += "      Turn " + m_Spot.Yaw.ToString() + ", " + m_Spot.Pitch.ToString() + ", " + m_Spot.Roll.ToString();
		m_Readout.SetText(readout);
		m_StepLabel.SetText("Step: " + (MOVE_STEPS[m_StepIndex] * 100).ToString() + " cm, " + TURN_STEPS[m_StepIndex].ToString() + " deg. Click to change");
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (w == m_Close)
		{
			Close();
			return true;
		}
		if (w == m_Save && m_Spot)
		{
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_VehicleLockNet.SAVE_SPOT, new Param1<string>(m_Spot.ToJson()), true, null);
			return true;
		}
		if (w == m_Step)
		{
			m_StepIndex = (m_StepIndex + 1) % 3;
			ShowSpot();
			return true;
		}
		if (!m_Spot)
			return false;

		float move = MOVE_STEPS[m_StepIndex];
		float turn = TURN_STEPS[m_StepIndex];
		string name = w.GetName();
		if (name == "XMinus")
			m_Spot.X -= move;
		else if (name == "XPlus")
			m_Spot.X += move;
		else if (name == "YMinus")
			m_Spot.Y -= move;
		else if (name == "YPlus")
			m_Spot.Y += move;
		else if (name == "ZMinus")
			m_Spot.Z -= move;
		else if (name == "ZPlus")
			m_Spot.Z += move;
		else if (name == "YawMinus")
			m_Spot.Yaw -= turn;
		else if (name == "YawPlus")
			m_Spot.Yaw += turn;
		else if (name == "PitchMinus")
			m_Spot.Pitch -= turn;
		else if (name == "PitchPlus")
			m_Spot.Pitch += turn;
		else if (name == "RollMinus")
			m_Spot.Roll -= turn;
		else if (name == "RollPlus")
			m_Spot.Roll += turn;
		else
			return false;
		ShowSpot();
		return true;
	}
}

modded class MissionGameplay
{
	protected ref RAPatch_LockPlacerMenu m_RAPatch_Placer;

	override void OnInit()
	{
		super.OnInit();
		RAPatch_VehicleLockNet.Get();
		RAPatch_VehicleLockNet.AskIfAdmin();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		CarScript car = RAPatch_VehicleLockNet.PlacerRequest;
		if (!car)
			return;
		RAPatch_VehicleLockNet.PlacerRequest = null;
		if (GetGame().GetUIManager().GetMenu())
			return;
		m_RAPatch_Placer = new RAPatch_LockPlacerMenu;
		GetGame().GetUIManager().ShowScriptedMenu(m_RAPatch_Placer, null);
		m_RAPatch_Placer.SetCar(car);
	}
}
