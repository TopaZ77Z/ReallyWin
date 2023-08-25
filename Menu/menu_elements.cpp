#include "menu_elements.h"
#include "menu.h"
#include "../Utils/Utils.h"
#include "../Utils/Animation.h"

#include <string>
#include <format>

std::string keyNames[] = { "",
	"M1", "M2", "CANCEL", "M3", "M4", "M5", "",
	"BACK", "TAB", "", "", "CLEAR", "ENTER", "", "",
	"SHF", "CONTROL", "ALT", "PAUSE", "CAPS", "", "", "", "", "", "",
	"ESCAPE", "", "", "", "", "SPACE", "PAGE UP", "PAGE DOWN",
	"END", "HOME", "LEFT", "UP", "RIGHT", "DOWN", "", "", "",
	"PRINT", "INS", "DEL", "",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"", "", "", "", "", "", "",
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K",
	"L", "M", "N", "O", "P", "Q", "R", "S", "T", "U",
	"V", "W", "X", "Y", "Z", "LWIN", "RWIN", "", "", "",
	"NUM0", "NUM1", "NUM2", "NUM3", "NUM4", "NUM5",
	"NUM6", "NUM7", "NUM8", "NUM9",
	"*", "+", "", "-", ".", "/",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
	"F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
	"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24",
	"", "", "", "", "", "", "", "",
	"NUM LOCK", "SCROLL LOCK",
	"", "", "", "", "", "", "",
	"", "", "", "", "", "", "",
	"LSHIFT", "RSHIFT", "LCTRL",
	"RCTRL", "LMENU", "RMENU"
};

void IBaseElement::set_visible(bool newVisible) {
	visible = newVisible;

	if (parentItem)
		parentItem->visible = newVisible;

	parent->Update();
}

void CCheckbox::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(90, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	Color colorActive = Menu->menuColor;
	Color colorDisabled = Color(77, 77, 77);

	anim = interpolate(anim, value ? 1.f : 0.f, 10);
	Color clr1 = colorActive.lerp(colorDisabled, 1.f - anim);
	Color clr2 = clr1 * 0.66f;

	Render->Box(_absPos + Vector2(2, 2), _absPos + Vector2(8, 8), Color(12, 12, 12));

	Render->GradientBox(_absPos + Vector2(2, 2), _absPos + Vector2(8, 8), clr1, clr1, clr2, clr2);

	Render->Text(name, _absPos + Vector2(20, -1), unsafe ? Color(164, 164, 120) : Color(200, 200, 200), Verdana);
}

void CCheckbox::HandleClick(bool rmb) {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(90, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	Vector2 clipRect[2] = { 
		parent->clipRect[0],
		parent->clipRect[1]
	};

	Vector2 hitbox[2] = {
		_absPos,
		_absPos + Vector2(100, 10)
	};

	Vector2* realhitbox = Utils::BoxIntersection(clipRect[0], clipRect[1], hitbox[0], hitbox[1]);

	if (Render->InBounds(realhitbox[0], realhitbox[1])) {
		value = !value;
		if (callback)
			callback();
	}
}

void CLabel::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(90, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);
	Render->Text(name, _absPos + Vector2(20, -1), Color(200, 200, 200), Verdana);
}

void CColorPicker::Draw() {
	if (!parentItem->visible || !visible)
		return;
	 
	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;
	Vector2 parentPos = groupPos + parentItem->pos - Vector2(0, parent->scrollOffset);

	Vector2 absPos(groupPos.x + parent->size.x - 36, parentPos.y);

	Render->BoxFilled(absPos, absPos + Vector2(16, 9), Color(12, 12, 12));
	
	Color clr1 = (color + Color(40, 40, 40)).alpha_modulate(255);
	Color clr2 = (color - Color(10, 10, 10)).alpha_modulate(255);

	Render->GradientBox(absPos + Vector2(1, 1), absPos + Vector2(15, 8), clr1, clr1, clr2, clr2);
}

void CColorPicker::HandleClick(bool rmb) {
	if (!parentItem->visible)
		return;

	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;
	Vector2 parentPos = groupPos + parentItem->pos - Vector2(0, parent->scrollOffset);

	Vector2 absPos(groupPos.x + parent->size.x - 36, parentPos.y);

	Vector2 clipRect[2] = {
		parent->clipRect[0],
		parent->clipRect[1]
	};

	Vector2 hitbox[2] = {
		absPos,
		absPos + Vector2(16, 8)
	};

	Vector2* realhitbox = Utils::BoxIntersection(clipRect[0], clipRect[1], hitbox[0], hitbox[1]);

	if (Render->InBounds(realhitbox[0], realhitbox[1]) && (!opened && !copyopened)) {
		if (rmb) {
			opened = false;
			copyopened = true;
			overlayPos = absPos - Vector2(105, -15);

			Render->PushDeadZone(overlayPos, overlayPos + Vector2(100, 40));
		}
		else {
			opened = true;
			copyopened = false;
			overlayPos = absPos + Vector2(0, 15);

			Render->PushDeadZone(overlayPos, overlayPos + Vector2(178, 173));
		}
	}
	else if (opened || copyopened) {
		Vector2 size = opened ? Vector2(175, 175) : Vector2(100, 50);

		if (!Render->InBounds(overlayPos, overlayPos + size, true)) {
			opened = copyopened = false;

			Render->PopDeadZone();
		}

		if (copyopened && !rmb) {
			if (Render->InBounds(overlayPos, overlayPos + Vector2(100, 20), true)) {
				Menu->copyColor = color;
				copyopened = false;

				Render->PopDeadZone();
			}
			else if (Render->InBounds(overlayPos + Vector2(0, 20), overlayPos + Vector2(100, 40), true)) {
				color = Menu->copyColor;
				copyopened = false;

				Render->PopDeadZone();
			}
		}
	}
}

void CColorPicker::DrawOverlay() {
	if (!parentItem->visible || !(opened || copyopened))
		return;

	if (opened) {
		Render->BoxFilled(overlayPos, overlayPos + Vector2(178, 173), Color(40, 40, 40));
		Render->Box(overlayPos, overlayPos + Vector2(178, 173), Color(12, 12, 12));
		Render->Box(overlayPos + Vector2(1, 1), overlayPos + Vector2(177, 172), Color(60, 60, 60));
		Render->Box(overlayPos + Vector2(4, 4), overlayPos + Vector2(154, 154), Color(12, 12, 12));

		Color topRight;
		topRight.FromHSV(h, 100, 100);

		Render->GradientBox(overlayPos + Vector2(4, 4), overlayPos + Vector2(154, 154), Color(255, 255, 255), topRight, Color(255, 255, 255), topRight);
		Render->GradientBox(overlayPos + Vector2(4, 4), overlayPos + Vector2(154, 154), Color(0, 0, 0, 0), Color(0, 0, 0, 0), Color(0, 0, 0), Color(0, 0, 0));

		Vector2 dotPos = overlayPos + Vector2(4, 4) + Vector2(s * 1.5f, 150 - v * 1.5f);

		Render->Box(dotPos - Vector2(1, 1), dotPos + Vector2(1, 1), Color(12, 12, 12));
		Render->BoxFilled(dotPos - Vector2(1, 1), dotPos + Vector2(1, 1), color.alpha_modulate(128));

		Render->Box(overlayPos + Vector2(4, 159), overlayPos + Vector2(154, 169), Color(12, 12, 12));
		Render->BoxFilled(overlayPos + Vector2(4, 159), overlayPos + Vector2(154, 169), (color * (color.a / 255.f)).alpha_modulate(255));
		Render->Box(overlayPos + Vector2(4 + color.a * 0.585f, 160), overlayPos + Vector2(7 + color.a * 0.585f, 168), Color(12, 12, 12));
		Render->BoxFilled(overlayPos + Vector2(5 + color.a * 0.585f, 160), overlayPos + Vector2(7 + color.a * 0.585f, 168), color.alpha_modulate(128));

		Render->Box(overlayPos + Vector2(159, 4), overlayPos + Vector2(174, 154), Color(12, 12, 12));

		for (int i = 0; i < 6; i++) {
			int h1 = 360 - i * 60;
			int h2 = h1 - 60;
			Color c1, c2;
			c1.FromHSV(h1);
			c2.FromHSV(h2);

			Render->GradientBox(overlayPos + Vector2(159, 5 + i * 60 * 0.415f), overlayPos + Vector2(174, 5 + (i * 60 + 60) * 0.415f), c1, c1, c2, c2);
		}

		if (GetAsyncKeyState(VK_LBUTTON) && !(holda || holdh || holdvs)) {
			if (Render->InBounds(overlayPos + Vector2(159, 4), overlayPos + Vector2(174, 154), true)) {
				holdh = true;
			}
			else if (Render->InBounds(overlayPos + Vector2(4, 159), overlayPos + Vector2(154, 169), true)) {
				holda = true;
			}
			else if (Render->InBounds(overlayPos + Vector2(4, 4), overlayPos + Vector2(154, 154), true)) {
				holdvs = true;
			}
		}
		else if (!GetAsyncKeyState(VK_LBUTTON)) {
			holdh = holda = holdvs = false;
		}

		if (holdh) {
			int offset = max(min(Render->GetMousePos().y - overlayPos.y - 4, 150), 0);
			h = 360 - offset * 2.4f;
			color.FromHSV(h, s, v, color.a);

			if (callback)
				callback();
		} else if (holda) {
			int offset = max(min(Render->GetMousePos().x - overlayPos.x - 4, 150), 0);
			color.a = offset * 1.7f;

			if (callback)
				callback();
		}
		else if (holdvs) {
			int offsetx = max(min(Render->GetMousePos().x - overlayPos.x - 4, 150), 0);
			int offsety = max(min(Render->GetMousePos().y - overlayPos.y - 4, 150), 0);
			v = 100 - offsety * 0.66f;
			s = offsetx * 0.66f;
			color.FromHSV(h, s, v, color.a);

			if (callback)
				callback();
		}

		Render->BoxFilled(overlayPos + Vector2(160, 4 + (360 - h) * 0.415f), overlayPos + Vector2(173, 6 + (360 - h) * 0.415f), Color(255, 255, 255, 128));
		Render->Box(overlayPos + Vector2(160, 4 + (360 - h) * 0.415f), overlayPos + Vector2(173, 6 + (360 - h) * 0.415f), Color(12, 12, 12));
	}
	else {
		Render->BoxFilled(overlayPos, overlayPos + Vector2(100, 40), Color(35, 35, 35));
		Render->Box(overlayPos, overlayPos + Vector2(100, 40), Color(12, 12, 12));

		int hover = -1;

		if (Render->InBounds(overlayPos, overlayPos + Vector2(100, 20), true)) {
			hover = 0;
		}
		else if (Render->InBounds(overlayPos + Vector2(0, 20), overlayPos + Vector2(100, 40), true)) {
			hover = 1;
		}

		if (hover != -1)
			Render->BoxFilled(overlayPos + Vector2(0, hover * 20), overlayPos + Vector2(100, 20 + hover * 20), Color(20, 20, 20));

		Render->Text("Copy", overlayPos + Vector2(10, 4), hover == 0 ? Menu->menuColor : Color(200, 200, 200), hover == 0 ? VerdanaBold : Verdana);
		Render->Text("Paste", overlayPos + Vector2(10, 24), hover == 1 ? Menu->menuColor : Color(200, 200, 200), hover == 1 ? VerdanaBold : Verdana);
	}
}

void CKeyBind::Draw() {
	if (!parentItem->visible)
		return;

	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;
	Vector2 parentPos = groupPos + parentItem->pos - Vector2(0, parent->scrollOffset);

	Vector2 absPos(groupPos.x + parent->size.x - 20, parentPos.y);

	std::string text = "[-]";

	if (key > 0 && key <= 166) {
		text = "[" + keyNames[key] + "]";
	}

	const int text_size = Render->CalcTextSize(text, SmallFont).x;

	Render->Text(text, absPos - Vector2(text_size, 0), bindState == 1 ? Color(200, 0, 0) : Color(120, 120, 120), SmallFont, TEXT_OUTLINED);
}

void CKeyBind::HandleClick(bool rmb) {
	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;
	Vector2 parentPos = groupPos + parentItem->pos - Vector2(0, parent->scrollOffset);

	Vector2 absPos(groupPos.x + parent->size.x - 20, parentPos.y);

	if (opened && !rmb) {
		for (int i = 0; i < 3; i++) {
			if (Render->InBounds(absPos - Vector2(105, -15 - 18.75f * i), absPos + Vector2(-5, 15 + 18.75f * (i + 1)), true)) {
				bindType = i;

				if (callback)
					callback();

				break;
			}
		}
	}

	if (!rmb && opened && !Render->InBounds(absPos - Vector2(105, -15), absPos + Vector2(-5, 90))) {
		opened = false;

		Render->PopDeadZone();
	}

	if (!Render->InBounds(absPos - Vector2(20, 0), absPos + Vector2(0, 10)) || !Render->InBounds(groupPos, groupPos + parent->size))
		return;

	if (rmb) {
		opened = true;

		Render->PushDeadZone(absPos, absPos + Vector2(100, 56));
	}
	else {
		bindState = 1;
	}
}

void CKeyBind::DrawOverlay() {
	if (!parentItem->visible || !opened)
		return;

	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;
	Vector2 parentPos = groupPos + parentItem->pos - Vector2(0, parent->scrollOffset);

	Vector2 absPos(groupPos.x + parent->size.x - 125, parentPos.y + 15);

	Render->BoxFilled(absPos, absPos + Vector2(100, 56), Color(35, 35, 35));
	Render->Box(absPos, absPos + Vector2(100, 56), Color(12, 12, 12));

	Render->Text("On hotkey", absPos + Vector2(10, 3), bindType == 0 ? Menu->menuColor : Color(200, 200, 200), Verdana);
	Render->Text("Toggle", absPos + Vector2(10, 22), bindType == 1 ? Menu->menuColor : Color(200, 200, 200), Verdana);
	Render->Text("Off hotkey", absPos + Vector2(10, 41), bindType == 2 ? Menu->menuColor : Color(200, 200, 200), Verdana);
}

bool CKeyBind::get() {
	if (key == -1)
		return false;

	switch (bindType)
	{
	case 0:
		return GetAsyncKeyState(key);
		break;
	case 1:
		if (GetAsyncKeyState(key) && !alreadyClicked) {
			alreadyClicked = true;
			toggled = !toggled;
		}
		else if (!GetAsyncKeyState(key)) {
			alreadyClicked = false;
		}
		return toggled;
		break;
	case 2:
		return !GetAsyncKeyState(key);
		break;
	}
}

void CKeyBind::HandleKeyPress(int vkKey) {
	if (bindState != 1)
		return;

	if (vkKey == VK_ESCAPE) {
		bindState = 0;
		key = -1;
	}
	else {
		key = vkKey;
		bindState = 2;
	}
}

void CSlider::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (!hideName)
		Render->Text(name, _absPos, Color(200, 200, 200), Verdana);
	else
		_absPos.y -= 15;

	Render->BoxFilled(_absPos + Vector2(-1, 15), _absPos + Vector2(201, 22), Color(12, 12, 12));
	Render->GradientBox(_absPos + Vector2(0, 16), _absPos + Vector2(200, 21), Color(68, 68, 68), Color(68, 68, 68), Color(52, 52, 52), Color(52, 52, 52));

	if (dragging) {
		float mouseOffset = max(min(Render->GetMousePos().x - _absPos.x, 200.f), 0.f);
		float fraction = mouseOffset / 200.f;

		value = fraction * (max - min) + min;

		if (callback)
			callback();
	}
	if (!GetAsyncKeyState(VK_LBUTTON)) {
		dragging = false;
	}

	int absValue = value / scale;
	float fraction = (float)(absValue - absMin) / (float)(absMax - absMin);

	Color clr1 = Menu->menuColor;
	Color clr2 = clr1 * 0.5f;

	Render->GradientBox(_absPos + Vector2(0, 16), _absPos + Vector2(200 * fraction, 21), clr1, clr1, clr2, clr2);

	if (scale == 1.f) {
		Render->Text(std::to_string((int)value) + unit, _absPos + Vector2(200 * fraction, 18), Color(200, 200, 200), VerdanaBold, TEXT_CENTERED | TEXT_OUTLINED);
	}
	else {
		int delimiter = 10;
		if (accuracy == 2)
			delimiter = 100;

		int num1 = absValue / delimiter;
		int num2 = (((float)absValue / (float)delimiter) - (float)num1) * delimiter;

		Render->Text(std::format("{}.{}{}", num1, num2, unit), _absPos + Vector2(200 * fraction, 18), Color(200, 200, 200), VerdanaBold, TEXT_CENTERED | TEXT_OUTLINED);
	}
}

void CSlider::HandleClick(bool rmb) {
	if (!visible || rmb)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 29) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (hideName)
		_absPos.y -= 15;

	if (Render->InBounds(_absPos, _absPos + Vector2(200, 15)))
		dragging = true;
}

void CComboBox::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (!hideName)
		Render->Text(name, _absPos, Color(200, 200, 200), Verdana);
	else
		_absPos.y -= 15;
	Render->Box(_absPos + Vector2(0, 15), _absPos + Vector2(200, 33), Color(12, 12, 12));

	Color clr1(31, 31, 31);
	Color clr2(36, 36, 36);

	if (opened) {
		clr1 = Color(41, 41, 41);
		clr2 = Color(46, 46, 46);
	}

	Render->GradientBox(_absPos + Vector2(0, 15), _absPos + Vector2(200, 33), clr1, clr1, clr2, clr2);
	Render->Text(items[value], _absPos + Vector2(10, 18), Color(200, 200, 200), Verdana);

	std::vector<Vector2> triangle{ _absPos + Vector2(190, 26), _absPos + Vector2(187, 24), _absPos + Vector2(193, 24) };
	Render->PolyFilled(triangle, Color(160, 160, 160));
}

void CComboBox::HandleClick(bool rmb) {
	if (rmb)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);
	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;

	if (!hideName)
		_absPos.y += 15;

	if (Render->InBounds(groupPos, groupPos + parent->size) && Render->InBounds(_absPos, _absPos + Vector2(200, 18))) {
		opened = !opened;

		if (opened)
			Render->PushDeadZone(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20));
		else
			Render->PopDeadZone();
	}
	else if (!Render->InBounds(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20), true) && opened) {
		opened = false;

		Render->PopDeadZone();
	}

	if (opened) {
		const Vector2 mofffset = Render->GetMousePos() - _absPos;

		if (mofffset.y < 21 || mofffset.x < 0 || mofffset.x > 200 || mofffset.y > 21 + items.size() * 20) {
			return;
		}

		const int i = (mofffset.y - 21) / 20;
		value = i;
		Render->PopDeadZone();
		opened = false;

		if (callback)
			callback();
	}
}

void CComboBox::DrawOverlay() {
	if (!opened)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);
	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;

	if (!hideName)
		_absPos.y += 15;

	Render->Box(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20), Color(12, 12, 12));
	Render->BoxFilled(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20), Color(35, 35, 35));

	for (int i = 0; i < items.size(); i++) {
		std::string item = items[i];

		Vector2 textPos = _absPos + Vector2(10, 25 + i * 20);
		D3DXFont* font = Verdana;

		if (Render->InBounds(_absPos + Vector2(0, 22 + i * 20), _absPos + Vector2(200, 41 + i * 20), true)) {
			font = VerdanaBold;
			Render->BoxFilled(_absPos + Vector2(0, 21 + i * 20), _absPos + Vector2(200, 41 + i * 20), Color(20, 20, 20));
		}

		if (value == i)
			font = VerdanaBold;

		Render->Text(item, textPos, value == i ? Menu->menuColor : Color(200, 200, 200), font);
	}
}

void CMultiCombo::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (!hideName)
		Render->Text(name, _absPos, Color(200, 200, 200), Verdana);
	else
		_absPos.y -= 15;
	Render->Box(_absPos + Vector2(0, 15), _absPos + Vector2(200, 33), Color(12, 12, 12));

	Color clr1(31, 31, 31);
	Color clr2(36, 36, 36);

	if (opened) {
		clr1 = Color(41, 41, 41);
		clr2 = Color(46, 46, 46);
	}

	std::string text;
	int totalAdded = 0;

	if (values == 0)
		text = "-";
	else {
		for (int i = 0; i < items.size(); i++) {
			if (values & (1 << i)) {
				std::string nextText;
				if (totalAdded == 0)
					nextText = items[i];
				else
				{
					nextText = text + ", " + items[i];
				}
				int textLength = Render->CalcTextSize(nextText, Verdana).x;


				if (textLength < 180) 
					text = nextText;
				else
				{
					if (totalAdded == 0)
						text = "...";
					else
						text += ", ...";
					break;
				}

				totalAdded++;
			}
		}
	}

	Render->GradientBox(_absPos + Vector2(0, 15), _absPos + Vector2(200, 33), clr1, clr1, clr2, clr2);
	Render->Text(text, _absPos + Vector2(10, 18), Color(200, 200, 200), Verdana);

	std::vector<Vector2> triangle{ _absPos + Vector2(190, 26), _absPos + Vector2(187, 24), _absPos + Vector2(193, 24) };
	Render->PolyFilled(triangle, Color(160, 160, 160));
}

void CMultiCombo::HandleClick(bool rmb) {
	if (rmb)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);
	Vector2 groupPos = Menu->menuPos + Vector2(90, 14) + parent->pos;

	if (!hideName)
		_absPos.y += 15;

	if (Render->InBounds(groupPos, groupPos + parent->size) && Render->InBounds(_absPos, _absPos + Vector2(200, 18))) {
		opened = !opened;

		if (opened)
			Render->PushDeadZone(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20));
		else
			Render->PopDeadZone();
	}
	else if (!Render->InBounds(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20), true) && opened) {
		opened = false;

		Render->PopDeadZone();
	}

	if (opened) {
		for (int i = 0; i < items.size(); i++) {
			if (Render->InBounds(_absPos + Vector2(0, 21 + i * 20), _absPos + Vector2(200, 40 + i * 20), true)) {
				values ^= 1 << i;

				if (callback)
					callback();
			}
		}
	}
}

void CMultiCombo::DrawOverlay() {
	if (!opened)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 10) + parent->pos + pos - Vector2(0, parent->scrollOffset);
	Vector2 groupPos = Menu->menuPos + Vector2(90, 10) + parent->pos;

	if (!hideName)
		_absPos.y += 15;

	Render->Box(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20), Color(12, 12, 12));
	Render->BoxFilled(_absPos + Vector2(0, 21), _absPos + Vector2(200, 21 + (int)items.size() * 20), Color(35, 35, 35));

	for (int i = 0; i < items.size(); i++) {
		std::string item = items[i];

		Vector2 textPos = _absPos + Vector2(10, 25 + i * 20);
		D3DXFont* font = Verdana;

		if (Render->InBounds(_absPos + Vector2(0, 21 + i * 20), _absPos + Vector2(200, 40 + i * 20), true)) {
			font = VerdanaBold;
			Render->BoxFilled(_absPos + Vector2(0, 21 + i * 20), _absPos + Vector2(200, 41 + i * 20), Color(20, 20, 20));
		}

		if (values & (1 << i))
			font = VerdanaBold;

		Render->Text(item, textPos, values & (1 << i) ? Menu->menuColor : Color(200, 200, 200), font);
	}
}

void CButton::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	Render->BoxFilled(_absPos, _absPos + Vector2(200, 25), Color(12, 12, 12));
	Render->BoxFilled(_absPos + Vector2(1, 1), _absPos + Vector2(199, 24), Color(50, 50, 50));

	Color gbColor1(34, 34, 34), gbColor2(30, 30, 30);

	if (Render->InBounds(_absPos, _absPos + Vector2(200, 25))) {
		if (GetAsyncKeyState(VK_LBUTTON)) {
			gbColor1 = Color(40, 40, 40);
			gbColor2 = Color(25, 25, 25);
		}
		else {
			gbColor1 = Color(40, 40, 40);
			gbColor2 = Color(36, 36, 36);
		}
	}

	Render->GradientBox(_absPos + Vector2(2, 2), _absPos + Vector2(198, 23), gbColor1, gbColor1, gbColor2, gbColor2);

	Render->Text(name, _absPos + Vector2(100, 6), Color(200, 200, 200), Verdana, TEXT_CENTERED | TEXT_DROPSHADOW);
}

void CButton::HandleClick(bool rmb) {
	if (!visible || rmb)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (Render->InBounds(_absPos, _absPos + Vector2(200, 25))) {
		if (callback)
			callback();
	}
}

void CInputBox::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (!hideName) {
		Render->Text(name, _absPos, Color(200, 200, 200), Verdana, TEXT_DROPSHADOW);
		_absPos.y += 15;
	}

	Render->BoxFilled(_absPos, _absPos + Vector2(200, 20), Color(12, 12, 12));
	Render->BoxFilled(_absPos + Vector2(1, 1), _absPos + Vector2(199, 19), Color(50, 50, 50));
	Render->BoxFilled(_absPos + Vector2(2, 2), _absPos + Vector2(198, 18), Color(25, 25, 25));

	Color textColor(200, 200, 200);
	
	if (focused) {
		textColor = Menu->menuColor;
	}

	Render->Text((input + '_'), _absPos + Vector2(5, 4), textColor, Verdana, TEXT_DROPSHADOW);
}

void CInputBox::HandleClick(bool rmb) {
	if (!visible || rmb)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (!hideName)
		_absPos.y += 15;

	focused = Render->InBounds(_absPos, _absPos + Vector2(200, 20));
}

void CInputBox::HandleInput(int key) {
	if (focused) {
		Utils::HandleInput(input, key);

		if (callback)
			callback();
	}
}

void CListBox::Draw() {
	if (!visible)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	if (hasfilter) {
		Render->BoxFilled(_absPos + Vector2(-1, -1), _absPos + Vector2(201, 321), Color(12, 12, 12));
		Render->BoxFilled(_absPos, _absPos + Vector2(200, 20), Color(50, 50, 50));
		Render->BoxFilled(_absPos + Vector2(1, 1), _absPos + Vector2(199, 19), Color(25, 25, 25));
		Render->BoxFilled(_absPos + Vector2(1, 21), _absPos + Vector2(199, 320), Color(35, 35, 35));

		Color textColor(200, 200, 200);

		if (focused) {
			textColor = Menu->menuColor;
		}

		Render->Text((filter + '_'), _absPos + Vector2(5, 4), textColor, Verdana, TEXT_DROPSHADOW);

		_absPos.y += 20;
	}
	else {
		Render->BoxFilled(_absPos + Vector2(-1, -1), _absPos + Vector2(201, 301), Color(12, 12, 12));
		Render->BoxFilled(_absPos + Vector2(0, 0), _absPos + Vector2(200, 300), Color(35, 35, 35));
	}

	Render->PushClipRect(_absPos, _absPos + Vector2(200, 300));

	int offset = 0;
	for (const auto& item : filtered_list()) {
		Color color(195, 195, 195);
		D3DXFont* font = Verdana;

		if (item == active) {
			color = Menu->menuColor;
			font = VerdanaBold;

			Render->BoxFilled(_absPos + Vector2(0, 5 + offset * 18 - scroll_offset), _absPos + Vector2(200, 25 + offset * 18 - scroll_offset), Color(26, 26, 26));
		}

		Render->Text(item, _absPos + Vector2(10, 10 + 18 * offset - scroll_offset), color, font);

		offset++;
	}
	offset++;

	Render->PopClipRect();

	const bool can_scroll = offset * 18 > 300;

	if (can_scroll) {
		const float scrollbar_scale = 300.f / (offset * 18.f);
		const int max_scroll = offset * 18 - 300;

		const int rel_scroll_offset = scroll_offset * scrollbar_scale;

		if (!GetAsyncKeyState(VK_LBUTTON)) {
			scrolling = false;
			scroll_mouse_offset = Render->GetMousePos().y - _absPos.y - rel_scroll_offset;
		}

		if (scrolling) {
			const int mouse_offset = Render->GetMousePos().y - _absPos.y - scroll_mouse_offset;
			scroll_offset = max(min(mouse_offset / scrollbar_scale, max_scroll), 0);
		}

		Render->BoxFilled(_absPos + Vector2(195, 0), _absPos + Vector2(200, 300), Color(40, 40, 40));
		Render->BoxFilled(_absPos + Vector2(196, rel_scroll_offset), _absPos + Vector2(200, rel_scroll_offset + 300 * scrollbar_scale), Color(65, 65, 65));
	}
	else {
		scroll_offset = 0;
	}
}

void CListBox::HandleClick(bool rmb) {
	if (!visible || rmb)
		return;

	Vector2 _absPos = Menu->menuPos + Vector2(110, 14) + parent->pos + pos - Vector2(0, parent->scrollOffset);

	focused = Render->InBounds(_absPos, _absPos + Vector2(200, 20));
	auto filtered = filtered_list();

	if (Render->InBounds(_absPos, _absPos + Vector2(180, 300))) {
		int m_offset = Render->GetMousePos().y - _absPos.y + scroll_offset - 5;
		int selected_index = m_offset / 18 - 1;

		if (selected_index >= filtered.size())
			return;

		active = filtered[selected_index];

		if (callback)
			callback();
	}

	if (filtered.size() * 18 > 300 && Render->InBounds(_absPos + Vector2(194, 0), _absPos + Vector2(200, 300)))
		scrolling = true;
}

void CListBox::HandleInput(int key) {
	if (focused)
		Utils::HandleInput(filter, key);
}

void CListBox::OnScroll(bool down) {
	const int size = (filtered_list().size() + 1) * 18;
	if (size < 300)
		return;

	const int max_scroll = size - 300;

	scroll_offset += down ? -10 : 10;

	scroll_offset = min(max(scroll_offset, 0), max_scroll);
}