#pragma once
#include "Interfaces.h"
#include "Misc/Color.h"
#include "Misc/Vector.h"
#include "Misc/Matrix.h"
#include <vector>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <any>
#include <d3d9.h>
#include <d3dx9.h>
#include <locale>
#include <codecvt>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

enum ETextFlags {
    TEXT_DROPSHADOW = (1 << 0),
    TEXT_OUTLINED = (1 << 1),
    TEXT_CENTERED = (1 << 2)
};

enum class EDrawType : int
{
    NONE = 0,
    PRIMITIVE,
    TEXT,
    IMAGE,
    CLIP,
    SETANTIALIAS
};

struct Vertex {
    float x = 0.f, y = 0.f, z = 0.f;
    DWORD color = 0xFFFFFFFF;

    Vertex() {}

    Vertex(float _x, float _y, Color clr) {
        x = _x;
        y = _y;
        color = D3DCOLOR_ARGB(clr.a, clr.r, clr.g, clr.b);
    }

    Vertex(const int _x, const int _y, Color clr) {
        x = (float)_x;
        y = (float)_y;
        color = D3DCOLOR_ARGB(clr.a, clr.r, clr.g, clr.b);
    }

    Vertex(const int _x, const int _y, DWORD clr) {
        x = (float)_x;
        y = (float)_y;
        color = clr;
    }

};

class D3DXFont {
public:
    ID3DXFont* font;
    ID3DXFont* _ts_font;

    void DrawTextA(ID3DXSprite* sprite, const char* text, RECT* rect, DWORD format, DWORD color) {
        font->DrawTextA(sprite, text, -1, rect, format, color);
    }

    void DrawTextW(ID3DXSprite* sprite, const wchar_t* text, RECT* rect, DWORD format, DWORD color) {
        font->DrawTextW(sprite, text, -1, rect, format, color);
    }

    Vector2 CalcTextSizeA(const char* text) {
        RECT rect(0);
        _ts_font->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT, 0);
        return Vector2(rect.right - rect.left, rect.bottom - rect.top);
    }

    Vector2 CalcTextSizeW(const wchar_t* text) {
        RECT rect(0);
        _ts_font->DrawTextW(NULL, text, -1, &rect, DT_CALCRECT, 0);
        return Vector2(rect.right - rect.left, rect.bottom - rect.top);
    }
};

struct primitive_command_t {
    D3DPRIMITIVETYPE primitiveType;
    UINT primitiveCount;
    std::vector<Vertex> pVertexData;

    primitive_command_t(D3DPRIMITIVETYPE primType, UINT primCount, std::vector<Vertex>& vertexData) :
        primitiveType(primType),
        primitiveCount(primCount),
        pVertexData(vertexData)
    {}
};

struct clip_command_t {
    RECT rect;
    bool remove = false;

    clip_command_t(RECT rec, bool rem = false) :
        rect(rec),
        remove(rem) 
    {}
};

struct text_command_t {
    std::string text;
    D3DXFont* font;
    RECT pRect;
    DWORD clipType;
    D3DCOLOR color;
    bool outlined = false;
    bool dropshadow = false;
};

struct image_command_t {
    LPDIRECT3DTEXTURE9 texture;
    RECT pSrcRect;
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    bool clip = false;

    image_command_t(LPDIRECT3DTEXTURE9 tex, RECT rect, D3DXVECTOR3 p, D3DCOLOR col, bool clp=false) :
        texture(tex),
        pSrcRect(rect),
        pos(p),
        color(col),
        clip(clp)
    {}
};

struct DrawCommand_t {
    DrawCommand_t(const EDrawType nType, std::any&& _object) :
        type(nType), object(std::move(_object)) { }

    EDrawType type;
    std::any object = {};
};

inline D3DXFont* Verdana;
inline D3DXFont* SmallFont;
inline D3DXFont* VerdanaBold;

namespace Resources {
    inline IDirect3DTexture9* Inferno;
    inline IDirect3DTexture9* HeGrenade;
    inline IDirect3DTexture9* Molotov;
}

class CRender
{
    IDirect3DDevice9* device;
    ID3DXSprite* sprite;
    DWORD oldFVF;

    std::vector<D3DXFont*> loaded_fonts;

    std::deque<DrawCommand_t> vecDrawData = { };
    std::deque<DrawCommand_t> vecSafeDrawData = { };
    std::shared_mutex drawMutex = { };

    ViewMatrix view_matrix;

    bool clipping = false;
    bool renderInitialized = false;
    DWORD nFonts;
    bool pop_deadzone_next_frame = false;
public:
    RECT clipRect;
    bool bDeadZone = false;
    Vector2 deadZone[2]{ Vector2(0, 0), Vector2(0, 0) };
   
    inline bool IsInitialized() { return renderInitialized; };

    void                Init(IDirect3DDevice9* dev);
    void                BeginFrame();
    void                RenderDrawData();
    void                EndFrame();
    void                Reset();
    void                SetAntiAliasing(bool value);

    void                BoxFilled(const Vector2& start, const Vector2& end, Color color, int rounding = 0);
    void                Box(const Vector2& start, const Vector2& end, Color color, int rounding = 0);
    void                GradientBox(const Vector2& start, const Vector2& end, Color color1, Color color2, Color color3, Color color4);
    void                Line(const Vector2& start, const Vector2& end, Color color);
    void                PolyLine(std::vector<Vector2> points, Color color);
    void                PolyFilled(std::vector<Vector2> points, Color color);
    void                Circle(const Vector2& center, float radius, Color color, int segments = -1, float begin = 0.f, float end = 360.f);
    void                CircleFilled(const Vector2& center, float radius, Color color, int segments = -1);
    void                Circle3D(const Vector& center, float radius, Color color, bool filled);
    void                Circle3DGradient(const Vector& center, float radius, Color color, bool reverse = false);
    void                GlowCircle(const Vector2& center, float radius, Color color);
    void                GlowCircle2(const Vector2& center, float radius, Color centerColor, Color edgeColor);

    void                AddFontFromMemory(void* file, unsigned int size);
    D3DXFont*           LoadFont(const std::string& fontname, int size, int weight = 400, int flags = CLEARTYPE_QUALITY);
    Vector2             CalcTextSize(const std::string& text, D3DXFont* font);
    void                Text(const std::string& text, const Vector2& pos, Color color, D3DXFont* font, int flags = 0);

    void                Text(const std::string& text, int x, int y, Color color, D3DXFont* font, int flags);

    IDirect3DTexture9*  LoadImageFromMemory(void* data, int dataSize, const Vector2& size);
    void                Image(IDirect3DTexture9* image, const Vector2& pos, Color color = Color(255, 255, 255));

    void                PushClipRect(const Vector2& start, const Vector2& end);
    void                PopClipRect();

    void                UpdateViewMatrix(const ViewMatrix& vm);
    Vector2             WorldToScreen(const Vector& vecPos);

    Vector2             GetMousePos();
    bool                InBounds(Vector2 start, Vector2 end, bool ignore_deadzone = false);
    void                PushDeadZone(Vector2 start, Vector2 end);
    void                PopDeadZone();
};

extern CRender* Render;