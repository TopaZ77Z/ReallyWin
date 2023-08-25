#include "menu.h"
#include "../Utils/Utils.h"

void CMenu::WndProc(UINT msg, WPARAM wParam) {
	if (!opened)
		return;

	switch (msg)
	{
	case WM_KEYDOWN:
		for (auto box : group_boxes) {
			if (box->tabId != activeTab)
				continue;

			for (auto element : box->elements) {
				if (element->GetItemType() == ElementType::KEYBIND)
					((CKeyBind*)(element))->HandleKeyPress(wParam);
				if (element->GetItemType() == ElementType::INPUTBOX)
					((CInputBox*)element)->HandleInput(wParam);
				if (element->GetItemType() == ElementType::LISTBOX)
					((CListBox*)element)->HandleInput(wParam);
			}
		}

		return;
	case WM_SYSKEYDOWN:
		if (wParam > 256)
			return;

		for (auto box : group_boxes) {
			if (box->tabId != activeTab)
				continue;

			for (auto element : box->elements)
				if (element->GetItemType() == ElementType::KEYBIND)
					((CKeyBind*)(element))->HandleKeyPress(wParam);
		}

		return;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		for (auto& box : group_boxes) {
			if (box->tabId == activeTab) {
				for (auto& element : box->elements) {
					if (element->visible)
						element->HandleClick(msg != WM_LBUTTONDOWN);
				}

				box->HandleClick();
			}
		}

		if (msg != WM_LBUTTONDOWN)
			return;

		if (Render->InBounds(menuPos + Vector2(8, 50), menuPos + Vector2(88, 705))) {
			activeTab = (Render->GetMousePos().y - menuPos.y - 50) / 80;
		}

		if (Render->InBounds(menuPos - Vector2(15, 15), menuPos + Vector2(size.x, 10)))
			dragging = true;

		if (Render->InBounds(menuPos + size - Vector2(25, 25), menuPos + size + Vector2(25, 25)))
			resizing = true;

		return;
	case WM_MOUSEWHEEL:
		for (auto& box : group_boxes) {
			if (box->tabId == activeTab) {
				box->HandleScroll((short)HIWORD(wParam) > 0);
			}

			for (const auto& item : box->elements) {
				if (item->GetItemType() == ElementType::LISTBOX)
					((CListBox*)item)->OnScroll((short)HIWORD(wParam) > 0);
			}
		}

		return;
	case WM_MBUTTONDOWN:
		for (auto box : group_boxes) {
			if (box->tabId != activeTab)
				continue;

			for (auto element : box->elements)
				if (element->GetItemType() == ElementType::KEYBIND)
					((CKeyBind*)(element))->HandleKeyPress(VK_MBUTTON);
		}

		return;
	case WM_XBUTTONDOWN:
		for (auto box : group_boxes) {
			if (box->tabId != activeTab)
				continue;

			for (auto element : box->elements)
				if (element->GetItemType() == ElementType::KEYBIND)
					((CKeyBind*)(element))->HandleKeyPress(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2);
		}

		return;
	}
	/*if (msg == WM_KEYDOWN) {
		if (opened) {
			for (auto& box : group_boxes) {
				if (box->tabId == activeTab) {
					for (auto& element : box->elements) {
						if (element->GetItemType() == ElementType::KEYBIND)
							((CKeyBind*)(element))->HandleKeyPress(wParam);
						if (element->GetItemType() == ElementType::INPUTBOX)
							((CInputBox*)element)->HandleInput(wParam);
						if (element->GetItemType() == ElementType::LISTBOX)
							((CListBox*)element)->HandleInput(wParam);
					}
				}
			}
		}
	}
	else if (msg == WM_LBUTTONDOWN) {
		if (opened) {
			for (auto& box : group_boxes) {
				if (box->tabId == activeTab) {
					for (auto& element : box->elements) {
						element->HandleClick(false);
					}

					box->HandleClick();
				}
			}

			if (Render->InBounds(menuPos + Vector2(8, 50), menuPos + Vector2(88, 705))) {
				activeTab = (Render->GetMousePos().y - menuPos.y - 50) / 80;
			}

			if (Render->InBounds(menuPos - Vector2(15, 15), menuPos + Vector2(size.x, 10)))
				dragging = true;

			if (Render->InBounds(menuPos + size - Vector2(25, 25), menuPos + size + Vector2(25, 25)))
				resizing = true;
		}
	}
	else if (msg == WM_RBUTTONDOWN) {
		if (opened) {
			for (auto& box : group_boxes) {
				if (box->tabId == activeTab) {
					for (auto& element : box->elements) {
						element->HandleClick(true);
					}

					box->HandleClick();
				}
			}
		}
	}
	else if (msg == WM_MOUSEWHEEL) {
		if (opened) {
			for (auto& box : group_boxes) {
				if (box->tabId == activeTab) {
					box->HandleScroll((short)HIWORD(wParam) > 0);
				}

				for (const auto& item : box->elements) {
					if (item->GetItemType() == ElementType::LISTBOX)
						((CListBox*)item)->OnScroll((short)HIWORD(wParam) > 0);
				}
			}
		}
	}*/
}

void CMenu::Draw() {
	if (!initialized)
		return;

	static Vector2 draggingOffest;

	opened = menu_key_bind->get();

	if (!opened)
		return;

	Vector2 mousePos = Render->GetMousePos();

	if (!GetAsyncKeyState(VK_LBUTTON)) {
		draggingOffest = Render->GetMousePos() - menuPos;
		dragging = resizing = false;
	}
	else if (dragging)
		menuPos = Render->GetMousePos() - draggingOffest;

	if (resizing)
		size = Render->GetMousePos() - menuPos;

	if (size.x < 720)
		size.x = 720;

	if (size.y < 660)
		size.y = 660;

	if (menuPos.x < -110)
		menuPos.x = -110;

	if (menuPos.y < -110)
		menuPos.y = -110;

	Menu->menuColor = menu_color_picker->get();

	Render->BoxFilled(menuPos, menuPos + size, Color(40, 40, 40));
	Render->Box(menuPos + Vector2(1, 1), menuPos + size - Vector2(1, 1), Color(62, 62, 62));
	Render->Box(menuPos + Vector2(5, 5), menuPos + size - Vector2(5, 5), Color(62, 62, 62));
	Render->Box(menuPos, menuPos + size, Color(12, 12, 12));
	Render->Box(menuPos + Vector2(6, 6), menuPos + Vector2(size.x - 6, 8), Color(12, 12, 12));
	Render->GradientBox(menuPos + Vector2(6, 6), menuPos + Vector2(size.x - 6 - (size.x - 16) / 2, 8), Color(93, 164, 187), Color(171, 96, 171), Color(93, 164, 187), Color(171, 96, 171));
	Render->GradientBox(menuPos + Vector2(size.x - 6 - (size.x - 16) / 2, 6), menuPos + Vector2(size.x - 6, 8), Color(171, 96, 171), Color(202, 213, 127), Color(171, 96, 171), Color(202, 213, 127));
	Render->PushClipRect(menuPos + Vector2(6, 9), menuPos + size - Vector2(6, 6));
	Render->Image(backgroundTexture, menuPos + Vector2(6, 9));
	Render->PopClipRect();
	Render->BoxFilled(menuPos + Vector2(6, 9), menuPos + Vector2(88, 49 + 80 * activeTab), Color(12, 12, 12));
	Render->BoxFilled(menuPos + Vector2(6, 131 + 80 * activeTab), menuPos + Vector2(88, size.y - 6), Color(12, 12, 12));
	Render->BoxFilled(menuPos + Vector2(88, 9), menuPos + Vector2(89, 49 + 80 * activeTab), Color(40, 40, 40));
	Render->BoxFilled(menuPos + Vector2(88, 131 + 80 * activeTab), menuPos + Vector2(89, size.y - 6), Color(40, 40, 40));

	Render->PushClipRect(menuPos + Vector2(90, 9), menuPos + Vector2(size.x - 6, size.y - 6));
	for (auto& group : group_boxes) {
		if (group->tabId == activeTab)
			group->Draw();
	}
	Render->PopClipRect();

	if (Menu->autoscaling && Menu->resizing) {
		for (auto group : group_boxes) {
			int columnOffset = 0;
			CGroupBox* upperbox = nullptr;

			for (auto groupbox : Menu->group_boxes) {
				if (groupbox->tabId == group->tabId && groupbox->column == group->column && groupbox != group && groupbox->pos.y < group->pos.y) {
					columnOffset += groupbox->size.y + 20;
					upperbox = groupbox;
				}
			}

			group->size.x = (Menu->size.x - 158) / 2.f;
			group->size.y = (Menu->size.y - 55) * group->scale - (upperbox ? 20 : 0);
			group->pos.x = group->column == 0 ? 20 : 40 + group->size.x;
			group->pos.y = 20 + columnOffset;
		}
	}

	for (auto& group : group_boxes) {
		if (group->tabId == activeTab) {
			for (auto element : group->elements) {
				switch (element->GetItemType())
				{
				case ElementType::COLORPCIKER:
					((CColorPicker*)element)->DrawOverlay();
					break;
				case ElementType::KEYBIND:
					((CKeyBind*)element)->DrawOverlay();
					break;
				case ElementType::COMBO:
					((CComboBox*)element)->DrawOverlay();
					break;
				case ElementType::MULTICOMBO:
					((CMultiCombo*)element)->DrawOverlay();
				default:
					break;
				}
			}
		}
	}

	Render->BoxFilled(menuPos + Vector2(6, 48 + 80 * activeTab), menuPos + Vector2(89, 49 + 80 * activeTab), Color(40, 40, 40));
	Render->BoxFilled(menuPos + Vector2(6, 130 + 80 * activeTab), menuPos + Vector2(89, 131 + 80 * activeTab), Color(40, 40, 40));
	Render->BoxFilled(menuPos + Vector2(6, 47 + 80 * activeTab), menuPos + Vector2(88, 48 + 80 * activeTab), Color(0, 0, 0));
	Render->BoxFilled(menuPos + Vector2(6, 131 + 80 * activeTab), menuPos + Vector2(88, 132 + 80 * activeTab), Color(0, 0, 0));
	Render->Image(aimImg, menuPos + Vector2(23, 65), Color(255, 255, 255, activeTab == 0 ? 255 : 120));
	Render->Image(antiaimImg, menuPos + Vector2(23, 145), Color(255, 255, 255, activeTab == 1 ? 255 : 120));
	Render->Image(visualsImg, menuPos + Vector2(23, 225), Color(255, 255, 255, activeTab == 2 ? 255 : 120));
	Render->Image(miscImg, menuPos + Vector2(23, 305), Color(255, 255, 255, activeTab == 3 ? 255 : 120));
	Render->Image(skinchangerImg, menuPos + Vector2(23, 385), Color(255, 255, 255, activeTab == 4 ? 255 : 120));
	Render->Image(playerListImg, menuPos + Vector2(23, 465), Color(255, 255, 255, activeTab == 5 ? 255 : 120));
	Render->Image(configImg, menuPos + Vector2(23, 545), Color(255, 255, 255, activeTab == 6 ? 255 : 120));

	if (size.y > 700)
		Render->Image(luaImg, menuPos + Vector2(23, 625), Color(255, 255, 255, activeTab == 7 ? 255 : 120));
}

CGroupBox* CMenu::AddGroupBox(const std::string& tab, const std::string& name, float scale, int column) {
	CGroupBox* groupBox = new CGroupBox;
	int tabId = get_tab_id(tab);

	int columnOffset = 0;
	CGroupBox* upperbox = nullptr;

	groupBox->tabId = tabId;

	for (auto groupbox : group_boxes) {
		if (groupBox->tabId == groupbox->tabId) {
			if (groupbox->column == column) {
				columnOffset += groupbox->size.y + 20;
				upperbox = groupbox;
			}
		}
	}

	if (upperbox)
		scale = 1 - upperbox->scale;

	groupBox->size.x = (size.x - 158) / 2.f;
	groupBox->size.y = (size.y - 55) * scale - (upperbox ? 20 : 0);
	groupBox->column = column;
	groupBox->scale = scale;
	groupBox->pos.x = groupBox->column == 0 ? 20 : 40 + groupBox->size.x;
	groupBox->pos.y = 20 + columnOffset;
	groupBox->name = name;
	groupBox->textSize = Render->CalcTextSize(name, VerdanaBold).x;

	group_boxes.push_back(groupBox);

	return groupBox;
}

void CGroupBox::Update() {
	minsize = 30;

	int next_position = 20;
	IBaseElement* last_item = nullptr;

	for (auto element : elements) {
		if (!element->visible)
			continue;

		if (!last_item) {
			element->pos = Vector2(20, 20);
		}
		else if (element->GetItemType() == ElementType::COLORPCIKER || element->GetItemType() == ElementType::KEYBIND) {
			element->pos = element->parentItem->pos;
		}
		else {
			element->pos = last_item->pos + Vector2(0, last_item->elementSize);
		}

		last_item = element;

		if (element->GetItemType() != ElementType::COLORPCIKER && element->GetItemType() != ElementType::KEYBIND)
			minsize += element->elementSize;
	}
}

CCheckbox* CMenu::AddCheckBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool unsafe) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += 18;

	CCheckbox* checkbox = new CCheckbox;

	checkbox->parent = target;
	checkbox->name = name;
	checkbox->elementSize = 18;
	checkbox->unsafe = unsafe;

	if (target->elements.empty())
		checkbox->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		checkbox->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(checkbox);

	return checkbox;
}

CLabel* CMenu::AddLabel(const std::string& tab, const std::string& groupbox, const std::string& name) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += 18;

	CLabel* label = new CLabel;

	label->parent = target;
	label->name = name;
	label->elementSize = 18;

	if (target->elements.empty())
		label->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		label->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(label);

	return label;
}

CColorPicker* CMenu::AddColorPicker(const std::string& tab, const std::string& groupbox, const std::string& name, Color defaultColor) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	IBaseElement* parentItem = nullptr;

	for (auto& element : target->elements) {
		if (element->name == name) {
			parentItem = element;
			break;
		}
	}

	if (!parentItem)
		parentItem = AddLabel(tab, groupbox, name);

	if (parentItem->parentItem)
		return nullptr;

	CColorPicker* colorpicker = new CColorPicker;

	colorpicker->parent = target;
	colorpicker->name = name;
	colorpicker->parentItem = parentItem;
	colorpicker->elementSize = parentItem->elementSize;
	colorpicker->color = defaultColor;
	colorpicker->pos = parentItem->pos;

	defaultColor.ToHSV(colorpicker->h, colorpicker->s, colorpicker->v);

	parentItem->parentItem = colorpicker;

	target->elements.push_back(colorpicker);

	return colorpicker;
}

CKeyBind* CMenu::AddKeyBind(const std::string& tab, const std::string& groupbox, const std::string& name, int defaultKey, int defaultType) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	IBaseElement* parentItem = nullptr;

	for (auto element : target->elements) {
		if (element->name == name) {
			parentItem = element;
			break;
		}
	}

	if (!parentItem)
		parentItem = AddLabel(tab, groupbox, name);

	if (parentItem->parentItem)
		return nullptr;

	if (parentItem->GetItemType() == ElementType::BUTTON)
		return nullptr;

	CKeyBind* keyBind = new CKeyBind;

	keyBind->parent = target;
	keyBind->name = name;
	keyBind->parentItem = parentItem;
	keyBind->elementSize = parentItem->elementSize;
	keyBind->pos = parentItem->pos;

	if (defaultKey != -1) {
		keyBind->key = defaultKey;
		keyBind->bindType = defaultType;
		keyBind->bindState = 2;
	}
	parentItem->parentItem = keyBind;

	target->elements.push_back(keyBind);

	return keyBind;
}

CSlider* CMenu::AddSlider(const std::string& tab, const std::string& groupbox, const std::string& name, float min, float max, float def, const std::string& unit, float scale, bool hideName) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += hideName ? 15 : 30;

	CSlider* slider = new CSlider;

	scale = min(max(0.01f, scale), 100);

	slider->name = name;
	slider->parent = target;
	slider->elementSize = hideName ? 15 : 30;
	slider->scale = scale;
	slider->max = max;
	slider->min = min;
	slider->absMin = min / scale;
	slider->absMax = max / scale;
	slider->value = def;
	slider->unit = unit;
	slider->hideName = hideName;

	if (scale == 1.f)
		slider->accuracy = 0;
	else if (scale < 0.1f)
		slider->accuracy = 2;
	else if (scale < 1.f)
		slider->accuracy = 1;

	if (target->elements.empty())
		slider->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements.back();
		slider->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(slider);

	return slider;
}

CComboBox* CMenu::AddComboBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, int def, bool hideName) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += !hideName ? 40 : 25;

	CComboBox* comboBox = new CComboBox;

	comboBox->name = name;
	comboBox->parent = target;
	comboBox->elementSize = !hideName ? 40 : 25;
	comboBox->items = items;
	comboBox->hideName = hideName;

	if (target->elements.empty())
		comboBox->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		comboBox->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(comboBox);

	return comboBox;
}

CMultiCombo* CMenu::AddMultiCombo(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, int def, bool hideName) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += !hideName ? 40 : 25;

	CMultiCombo* comboBox = new CMultiCombo;

	comboBox->name = name;
	comboBox->parent = target;
	comboBox->elementSize = !hideName ? 40 : 25;
	comboBox->items = items;
	comboBox->hideName = hideName;

	if (target->elements.empty())
		comboBox->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		comboBox->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(comboBox);

	return comboBox;
}

CInputBox* CMenu::AddInputBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool hideName) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += !hideName ? 40 : 25;

	CInputBox* inputBox = new CInputBox;

	inputBox->name = name;
	inputBox->parent = target;
	inputBox->elementSize = !hideName ? 40 : 25;
	inputBox->hideName = hideName;

	if (target->elements.empty())
		inputBox->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		inputBox->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(inputBox);

	return inputBox;
}

CButton* CMenu::AddButton(const std::string& tab, const std::string& groupbox, const std::string& name) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += 33;

	CButton* button = new CButton;

	button->name = name;
	button->parent = target;
	button->elementSize = 33;

	if (target->elements.empty())
		button->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		button->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(button);

	return button;
}

CListBox* CMenu::AddListBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, bool filter) {
	CGroupBox* target = nullptr;
	int tabId = get_tab_id(tab);

	for (auto gb : group_boxes) {
		if (gb->tabId == tabId && gb->name == groupbox) {
			target = gb;
			break;
		}
	}

	if (!target)
		return nullptr;

	target->minsize += 37;

	CListBox* listbox = new CListBox;

	listbox->name = name;
	listbox->parent = target;
	listbox->elementSize = filter ? 335 : 315;
	listbox->items = items;
	listbox->hasfilter = filter;
	listbox->active = "";

	if (target->elements.empty())
		listbox->pos = Vector2(20, 20);
	else {
		IBaseElement* lastItem = target->elements[target->elements.size() - 1];
		listbox->pos = lastItem->pos + Vector2(0, lastItem->elementSize);
	}

	target->elements.push_back(listbox);

	return listbox;
}

void CGroupBox::Draw() {
	Vector2 origin = Menu->menuPos + Vector2(90, 10) + pos;

	Color colorRect(40, 40, 40);
	Color colorText(210, 210, 210);

	if ((Menu->transformingGroup && Menu->transformingGroup != this) || scrolling || !GetAsyncKeyState(VK_LBUTTON)) {
		dragging = resizing = false;
	}

	if (dragging || resizing) {
		Menu->autoscaling = false;
		Menu->transformingGroup = this;
	}
	else {
		Menu->transformingGroup = nullptr;
	}

	if (dragging)
		colorText = Menu->menuColor;
	else if (resizing)
		colorRect = Menu->menuColor;

	bool canScroll = size.y < minsize;

	Render->BoxFilled(origin, origin + size, Color(23, 23, 23));
	Render->BoxFilled(origin - Vector2(1, 1), origin + Vector2(12, 0), colorRect);
	Render->BoxFilled(origin - Vector2(1, 1), origin + Vector2(0, size.y + 1), colorRect);
	Render->BoxFilled(origin + Vector2(-1, size.y), origin + Vector2(size.x, size.y + 1), colorRect);
	Render->BoxFilled(origin + Vector2(size.x, -1), origin + Vector2(size.x + 1, size.y + 1), colorRect);
	Render->BoxFilled(origin + Vector2(15 + textSize + 3, -1), origin + Vector2(size.x + 1, 0), colorRect);
	Render->BoxFilled(origin - Vector2(2, 2), origin + Vector2(12, -1), Color(12, 12, 12));
	Render->BoxFilled(origin - Vector2(2, 2), origin + Vector2(-1, size.y + 2), Color(12, 12, 12));
	Render->BoxFilled(origin + Vector2(-2, size.y + 1), origin + Vector2(size.x + 1, size.y + 2), Color(12, 12, 12));
	Render->BoxFilled(origin + Vector2(size.x + 1, -2), origin + Vector2(size.x + 2, size.y + 2), Color(12, 12, 12));
	Render->BoxFilled(origin + Vector2(15 + textSize + 3, -2), origin + Vector2(size.x + 2, -1), Color(12, 12, 12));

	std::vector<Vector2> triangle{ origin + size + Vector2(-5, 0), origin + size + Vector2(0, -5), origin + size };

	RECT oldClipRect = Render->clipRect;
	Vector2* newClipRect = Utils::BoxIntersection(Vector2((float)oldClipRect.left, (float)oldClipRect.top), Vector2((float)oldClipRect.right, (float)oldClipRect.bottom), origin, origin + size);

	clipRect[0] = newClipRect[0];
	clipRect[1] = newClipRect[1];

	Render->PopClipRect();
	Render->PushClipRect(clipRect[0], clipRect[1]);

	for (auto element : elements) {
		element->parent = this;
		element->Draw();
	}

	Render->PopClipRect();
	Render->PushClipRect(Vector2((float)oldClipRect.left, (float)oldClipRect.top), Vector2((float)oldClipRect.right, (float)oldClipRect.bottom));

	Render->GradientBox(origin, origin + Vector2(size.x, 20), Color(23, 23, 23), Color(23, 23, 23), Color(23, 23, 23, 0), Color(23, 23, 23, 0));
	Render->GradientBox(origin + Vector2(0, size.y - 20), origin + size, Color(23, 23, 23, 0), Color(23, 23, 23, 0), Color(23, 23, 23), Color(23, 23, 23));

	Render->PolyFilled(triangle, colorRect);

	if (canScroll) {
		float scrollBarScale = size.y / minsize;
		int maxScroll = (1.f - scrollBarScale) * minsize;
		bool canScrollUp = scrollOffset > 0;
		bool canScrollDown = scrollOffset < maxScroll;

		int relScrollOffset = scrollOffset * scrollBarScale;

		if (!GetAsyncKeyState(VK_LBUTTON)) {
			scrolling = false;
			scrollMouseOffset = Render->GetMousePos().y - (origin.y + relScrollOffset);
		}

		if (Menu->transformingGroup)
			scrolling = false;

		if (scrolling) {
			int mouseOffset = Render->GetMousePos().y - origin.y - scrollMouseOffset;
			scrollOffset = max(min(mouseOffset / scrollBarScale, maxScroll), 0);
		}

		if (canScrollUp) {
			std::vector<Vector2> downArrow{ origin + Vector2(size.x - 13, 6), origin + Vector2(size.x - 16, 9), origin + Vector2(size.x - 10, 9) };
			Render->BoxFilled(origin + Vector2(size.x - 15, 9), origin + Vector2(size.x - 11, 10), Color(0, 0, 0));
			Render->PolyFilled(downArrow, Color(205, 205, 205));
		}

		if (canScrollDown) {
			std::vector<Vector2> downArrow{origin + Vector2(size.x - 13, size.y - 6), origin + Vector2(size.x - 16, size.y - 9), origin + Vector2(size.x - 10, size.y - 9) };
			Render->BoxFilled(origin + Vector2(size.x - 15, size.y - 10), origin + Vector2(size.x - 11, size.y - 9), Color(0, 0, 0));
			Render->PolyFilled(downArrow, Color(205, 205, 205));
		}

		Render->BoxFilled(origin + Vector2(size.x - 5, 0), origin + size, Color(40, 40, 40));
		Render->BoxFilled(origin + Vector2(size.x - 4, relScrollOffset), origin + Vector2(size.x, relScrollOffset + size.y * scrollBarScale), Color(65, 65, 65));

		scrollOffset = max(min(scrollOffset, maxScroll), 0);
	}
	else {
		scrollOffset = 0;
	}

	Render->Text(name, origin + Vector2(15, -7), colorText, VerdanaBold);

	if (dragging) {
		Vector2 relpos = Render->GetMousePos() - (Menu->menuPos + Vector2(130, 10));

		pos = Vector2(int(relpos.x / 20) * 20, int(relpos.y / 20) * 20);

		if (pos.x < 10)
			pos.x = 10;
		if (pos.y < 10)
			pos.y = 10;
	}

	if (resizing) {
		Vector2 relpos = Render->GetMousePos() - (Menu->menuPos + Vector2(90, 10));

		size = Vector2(int(relpos.x / 10) * 10, int(relpos.y / 10) * 10) - pos;

		if (size.x < 100)
			size.x = 100;
		if (size.y < 50)
			size.y = 50;
	}
}

void CGroupBox::HandleClick() {
	Vector2 origin = Menu->menuPos + Vector2(90, 10) + pos;

	bool canScroll = size.y < minsize;

	if (canScroll) {
		if (GetAsyncKeyState(VK_LBUTTON) && Render->InBounds(origin + Vector2(size.x - 6, 0), origin + size)) {
			scrolling = true;
		}

		float scrollBarScale = size.y / minsize;
		int maxScroll = (1.f - scrollBarScale) * minsize;

		bool canScrollUp = scrollOffset > 0;
		bool canScrollDown = scrollOffset < maxScroll;
		if (Render->InBounds(origin + Vector2(size.x - 20, 5), origin + Vector2(size.x - 8, 15)))
			scrollOffset -= 10;

		if (Render->InBounds(origin + Vector2(size.x - 20, size.y - 15), origin + Vector2(size.x - 8, size.y - 5)))
			scrollOffset += 10;

		scrollOffset = max(min(scrollOffset, maxScroll), 0);
	}

	if (Render->InBounds(origin + Vector2(15, -7), origin + Vector2(20 + textSize, 7)))
		dragging = true;

	if (Render->InBounds(origin + Vector2(size.x - 5, size.y - 5), origin + Vector2(size.x + 5, size.y + 5)))
		resizing = true;
}

void CGroupBox::HandleScroll(bool up) {
	Vector2 origin = Menu->menuPos + Vector2(90, 14) + pos;

	bool canScroll = size.y < minsize;

	if (canScroll && Render->InBounds(origin, origin + size)) {
		float scrollBarScale = size.y / minsize;
		int maxScroll = (1.f - scrollBarScale) * minsize;
		scrollOffset += up ? -20 : 20;

		scrollOffset = max(min(scrollOffset, maxScroll), 0);
	}
}

CMenu* Menu = new CMenu;