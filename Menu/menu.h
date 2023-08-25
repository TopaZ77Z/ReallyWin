#pragma once
#include "../SDK/Render.h"
#include "menu_elements.h"

class CGroupBox {
public:
	Vector2 pos;
	Vector2 size;
	Vector2 clipRect[2];
	std::string name;
	int tabId;
	int column;
	int scrollOffset = 0;
	float textSize;
	float scale = 0;
	int minsize = 30;

	bool dragging = false;
	bool resizing = false;
	bool scrolling = false;

	int scrollMouseOffset = 0;

	std::vector<IBaseElement*> elements;

	void Draw();
	void HandleClick();
	void HandleScroll(bool up);
	void Update();
};

class CMenu
{
private:
	bool opened = true;
	bool dragging = false;

	int activeTab = 0;

	IDirect3DTexture9* aimImg;
	IDirect3DTexture9* visualsImg;
	IDirect3DTexture9* backgroundTexture;
	IDirect3DTexture9* miscImg;
	IDirect3DTexture9* antiaimImg;
	IDirect3DTexture9* skinchangerImg;
	IDirect3DTexture9* configImg;
	IDirect3DTexture9* playerListImg;
	IDirect3DTexture9* luaImg;

	inline int get_tab_id(const std::string& name) {
		if (name == "RAGE")
			return 0;
		else if (name == "ANTIAIM")
			return 1;
		else if (name == "VISUALS")
			return 2;
		else if (name == "MISC")
			return 3;
		else if (name == "SKINS")
			return 4;
		else if (name == "PLAYERS")
			return 5;
		else if (name == "CONFIG")
			return 6;
		else if (name == "LUA")
			return 7;
		return 0;
	};
public:
	bool initialized = false;
	CColorPicker* menu_color_picker;
	CKeyBind* menu_key_bind;
	std::vector<CGroupBox*> group_boxes;
	Vector2 menuPos{ 200, 200 };
	Vector2 size{ 760, 710 };
	Color menuColor{ 150, 210, 25 };
	bool autoscaling{ true };
	CGroupBox* transformingGroup{ nullptr };
	Vector2 deadZoneStart;
	Vector2 deadZoneEnd;
	bool isInDeadZone{ false };
	bool resizing{ false };

	Color copyColor;

	inline bool is_opened() { return opened; };

	void Init();
	void WndProc(UINT msg, WPARAM wParam);

	void Draw();
	CGroupBox*		AddGroupBox(const std::string& tab, const std::string& name, float scale, int column);
	CCheckbox*		AddCheckBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool unsafe = false);
	CLabel*			AddLabel(const std::string& tab, const std::string& groupbox, const std::string& name);
	CColorPicker*	AddColorPicker(const std::string& tab, const std::string& groupbox, const std::string& name, Color defaultColor = Color(255, 255, 255));
	CKeyBind*		AddKeyBind(const std::string& tab, const std::string& groupbox, const std::string& name, int defaultKey = -1, int defaultType = 1);
	CSlider*		AddSlider(const std::string& tab, const std::string& groupbox, const std::string& name, float min, float max, float def, const std::string& unit = "", float scale = 1.f, bool hideName = false);
	CComboBox*		AddComboBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, int def = 0, bool hideName = false);
	CMultiCombo*	AddMultiCombo(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, int def = 0, bool hideName = false);
	CButton*		AddButton(const std::string& tab, const std::string& groupbox, const std::string& name);
	CInputBox*		AddInputBox(const std::string& tab, const std::string& groupbox, const std::string& name, bool hidden = false);
	CListBox*		AddListBox(const std::string& tab, const std::string& groupbox, const std::string& name, std::vector<std::string> items, bool filter = true);
};

extern CMenu* Menu;