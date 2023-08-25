#pragma once
#include "../SDK/Render.h"

class CGroupBox;

enum ElementType : int {
	CHECKBOX,
	LABEL,
	COLORPCIKER,
	KEYBIND,
	SLIDER,
	COMBO,
	MULTICOMBO,
	BUTTON,
	INPUTBOX,
	LISTBOX
};

typedef void(*t_menu_callback)();

class IBaseElement
{
public:
	CGroupBox* parent;
	Vector2 pos; // groupbox relative!
	std::string name;
	float anim = 0.f;
	int elementSize = 0;
	bool visible = true;
	t_menu_callback callback = nullptr;
	IBaseElement* parentItem = nullptr;
	bool unsafe = false;

	virtual ElementType    GetItemType() = 0;
	virtual void	Draw() = 0;
	virtual void	HandleClick(bool rmb) = 0;

	void	set_visible(bool newVisible);
	void	set_callback(t_menu_callback p_callback) { 
		callback = p_callback; 
		if (this->GetItemType() != ElementType::BUTTON)
			callback(); 
	};
};

class CKeyBind : public IBaseElement {
public:
	int bindType = 0;
	int bindState = 0;
	int key = -1;
	bool opened = false;
	bool alreadyClicked = false;
	bool toggled = false;

	virtual ElementType    GetItemType() { return ElementType::KEYBIND; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	void			DrawOverlay();
	bool			get();
	void			HandleKeyPress(int vkKey);
};

class CCheckbox : public IBaseElement {
public:
	bool value = false;

	virtual ElementType    GetItemType() { return ElementType::CHECKBOX; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	inline bool			get() {
		if (!parentItem || parentItem->GetItemType() == ElementType::COLORPCIKER)
			return value;

		return ((CKeyBind*)parentItem)->get() && value;
	}
};

class CLabel : public IBaseElement {
public:
	virtual ElementType    GetItemType() { return ElementType::LABEL; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb) {};
};

class CColorPicker : public IBaseElement {
public:
	Color color;
	bool opened = false;
	bool copyopened = false;
	Vector2 overlayPos;
	int h = 0, s = 0, v = 100;
	bool holdh = false, holdvs = false, holda = false;

	virtual ElementType    GetItemType() { return ElementType::COLORPCIKER; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	void			DrawOverlay();
	inline Color			get() { return color; };

	void			Set(Color col) {
		col.ToHSV(h, s, v);
		color = col;
	};

	void			Set(int col32) {
		Color col;
		col.as_int32(col32);
		col.ToHSV(h, s, v);
		color = col;
	};
};

class CSlider : public IBaseElement {
public:
	float value = 0;
	float scale = 1.f;
	float min;
	float max;
	int absMin;
	int absMax;
	std::string unit;
	int accuracy = 0;
	bool dragging = false;
	bool hideName = false;

	virtual ElementType    GetItemType() { return ElementType::SLIDER; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	inline float			get() { return value; };
};

class CComboBox : public IBaseElement {
public:
	std::vector<std::string> items;
	bool opened = false;
	int value = 0;
	bool hideName = false;

	virtual ElementType    GetItemType() { return ElementType::COMBO; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	void			DrawOverlay();
	inline int		get() { return value; };
	void			UpdateList(std::vector<std::string> newItems) { items = newItems; };
};

class CMultiCombo : public IBaseElement {
public:
	std::vector<std::string> items;
	bool opened = false;
	int values = 0;
	bool hideName = false;

	virtual ElementType    GetItemType() { return ElementType::MULTICOMBO; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	void			DrawOverlay();
	inline bool			get(int index) { return values & (1 << index); };
	void			UpdateList(std::vector<std::string> newItems) { items = newItems; };
};

class CButton : public IBaseElement {
public:
	virtual ElementType    GetItemType() { return ElementType::BUTTON; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
};

class CInputBox : public IBaseElement {
public:
	std::string input;
	bool focused = false;
	bool hideName = false;

	virtual ElementType    GetItemType() { return ElementType::INPUTBOX; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	inline std::string&	get() { return input; };
	void			HandleInput(int vkey);
};

class CListBox : public IBaseElement {
	std::vector<std::string> filtered_list() {
		if (filter == "")
			return items;

		std::vector<std::string> result;

		for (const auto& item : items) {
			if (item.find(filter) != std::string::npos)
				result.emplace_back(item);
		}

		return result;
	}

	bool scrolling = false;
	int scroll_mouse_offset = 0;
	int scroll_offset = 0;
	bool focused = false;
public:
	std::vector<std::string> items;
	std::string active;
	bool hasfilter = true;
	std::string filter;

	virtual ElementType    GetItemType() { return ElementType::LISTBOX; };
	virtual void	Draw();
	virtual void	HandleClick(bool rmb);
	inline std::string		get() { return active; };
	void			UpdateList(std::vector<std::string> newItems) { items = newItems; };
	void			HandleInput(int key);
	void			OnScroll(bool down);
};