#pragma once


struct VPANEL;

class IPanel
{
public:
	virtual void Init(VPANEL vguiPanel, void* panel) = 0;
	virtual void null();
	virtual void SetPos(VPANEL vguiPanel, int x, int y) = 0;
	virtual void GetPos(VPANEL vguiPanel, int& x, int& y) = 0;
	virtual void SetSize(VPANEL vguiPanel, int wide, int tall) = 0;
	virtual void GetSize(VPANEL vguiPanel, int& wide, int& tall) = 0;
	virtual void SetMinimumSize(VPANEL vguiPanel, int wide, int tall) = 0;
	virtual void GetMinimumSize(VPANEL vguiPanel, int& wide, int& tall) = 0;
	virtual void SetZPos(VPANEL vguiPanel, int z) = 0;
	virtual int  GetZPos(VPANEL vguiPanel) = 0;
	virtual void GetAbsPos(VPANEL vguiPanel, int& x, int& y) = 0;
	virtual void GetClipRect(VPANEL vguiPanel, int& x0, int& y0, int& x1, int& y1) = 0;
	virtual void SetInset(VPANEL vguiPanel, int left, int top, int right, int bottom) = 0;
	virtual void GetInset(VPANEL vguiPanel, int& left, int& top, int& right, int& bottom) = 0;
	virtual void SetVisible(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsVisible(VPANEL vguiPanel) = 0;
	virtual void SetParent(VPANEL vguiPanel, VPANEL newParent) = 0;
	virtual int GetChildCount(VPANEL vguiPanel) = 0;
	virtual VPANEL GetChild(VPANEL vguiPanel, int index) = 0;
	virtual void GetChildren(VPANEL vguiPanel) = 0;
	virtual VPANEL GetParent(VPANEL vguiPanel) = 0;
	virtual void MoveToFront(VPANEL vguiPanel) = 0;
	virtual void MoveToBack(VPANEL vguiPanel) = 0;
	virtual bool HasParent(VPANEL vguiPanel, VPANEL potentialParent) = 0;
	virtual bool IsPopup(VPANEL vguiPanel) = 0;
	virtual void SetPopup(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsFullyVisible(VPANEL vguiPanel) = 0;
	virtual int GetScheme(VPANEL vguiPanel) = 0;
	virtual bool IsProportional(VPANEL vguiPanel) = 0;
	virtual bool IsAutoDeleteSet(VPANEL vguiPanel) = 0;
	virtual void DeletePanel(VPANEL vguiPanel) = 0;
	virtual void SetKeyBoardInputEnabled(VPANEL vguiPanel, bool state) = 0;
	virtual void SetMouseInputEnabled(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsKeyBoardInputEnabled(VPANEL vguiPanel) = 0;
	virtual bool IsMouseInputEnabled(VPANEL vguiPanel) = 0;
	virtual void Solve(VPANEL vguiPanel) = 0;
	virtual const char* GetName(unsigned int vguiPanel) = 0;
	virtual const char* GetClassName(VPANEL vguiPanel) = 0;

	// delivers a message to the panel
	virtual void SendMessage(VPANEL vguiPanel, void* params, VPANEL ifromPanel) = 0;

	// these pass through to the IClientPanel
	virtual void Think(VPANEL vguiPanel) = 0;
	virtual void PerformApplySchemeSettings(VPANEL vguiPanel) = 0;
	virtual void PaintTraverse(VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) = 0;
	virtual void Repaint(VPANEL vguiPanel) = 0;
	virtual VPANEL IsWithinTraverse(VPANEL vguiPanel, int x, int y, bool traversePopups) = 0;
	virtual void OnChildAdded(VPANEL vguiPanel, VPANEL child) = 0;
	virtual void OnSizeChanged(VPANEL vguiPanel, int newWide, int newTall) = 0;

	virtual void InternalFocusChanged(VPANEL vguiPanel, bool lost) = 0;
	virtual bool RequestInfo(VPANEL vguiPanel, void* outputData) = 0;
	virtual void RequestFocus(VPANEL vguiPanel, int direction = 0) = 0;
	virtual bool RequestFocusPrev(VPANEL vguiPanel, VPANEL existingPanel) = 0;
	virtual bool RequestFocusNext(VPANEL vguiPanel, VPANEL existingPanel) = 0;
	virtual VPANEL GetCurrentKeyFocus(VPANEL vguiPanel) = 0;
	virtual int GetTabPosition(VPANEL vguiPanel) = 0;

	// used by ISurface to store platform-specific data
	virtual void* Plat(VPANEL vguiPanel) = 0;
	virtual void SetPlat(VPANEL vguiPanel, void* Plat) = 0;

	// returns a pointer to the vgui controls baseclass Panel *
	// destinationModule needs to be passed in to verify that the returned Panel * is from the same module
	// it must be from the same module since Panel * vtbl may be different in each module
	virtual IPanel* GetPanel(VPANEL vguiPanel, const char* destinationModule) = 0;

	virtual bool IsEnabled(VPANEL vguiPanel) = 0;
	virtual void SetEnabled(VPANEL vguiPanel, bool state) = 0;

	// Used by the drag/drop manager to always draw on top
	virtual bool IsTopmostPopup(VPANEL vguiPanel) = 0;
	virtual void SetTopmostPopup(VPANEL vguiPanel, bool state) = 0;

	// sibling pins
	virtual void SetSiblingPin(VPANEL vguiPanel, VPANEL newSibling, char iMyCornerToPin = 0, char iSiblingCornerToPinTo = 0) = 0;

};