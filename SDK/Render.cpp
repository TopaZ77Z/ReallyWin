#include "Render.h"
#include "Globals.h"
#include "../Utils/Utils.h"
#include "../Resources/Fonts.h"

#include "../Resources/inferno.h"
#include "../Resources/molotov.h"
#include "../Resources/hegrenade.h"

#include <vector>
#include <mutex>

DWORD colorwrite, srgbwrite;
IDirect3DVertexDeclaration9* vert_dec = nullptr;
IDirect3DVertexShader9* vert_shader = nullptr;
DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
DWORD m_old_fvf;

struct Vec4 {
	float x, y, z, w;
};

std::wstring utf8_convert(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
	std::wstring result(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &result[0], size_needed);
	return result;
}

void CRender::Init(IDirect3DDevice9* dev) {
	device = dev;
	D3DXCreateSprite(device, &sprite);

	auto creationParams = D3DDEVICE_CREATION_PARAMETERS();
	device->GetCreationParameters(&creationParams);

	D3DVIEWPORT9 vp;
	device->GetViewport(&vp);
	Cheat.ScreenSize.x = vp.Width;
	Cheat.ScreenSize.y = vp.Height;

	AddFontFromMemory(FontFiles::MuseoSansCyrl, sizeof(FontFiles::MuseoSansCyrl));

	Verdana = LoadFont("Verdana", 12, 400, CLEARTYPE_NATURAL_QUALITY);
	SmallFont = LoadFont("Small Fonts", 8, 400, 0);
	VerdanaBold = LoadFont("Verdana Bold", 12, 600, CLEARTYPE_NATURAL_QUALITY);

	Resources::Inferno = Render->LoadImageFromMemory(inferno_icon, sizeof(inferno_icon), Vector2(30, 29));
	Resources::Molotov = Render->LoadImageFromMemory(molotov_icon, sizeof(molotov_icon), Vector2(19, 32));
	Resources::HeGrenade = Render->LoadImageFromMemory(hegrenade_icon, sizeof(hegrenade_icon), Vector2(19, 32));

	renderInitialized = true;
}

void CRender::BeginFrame() {
	if (!vecDrawData.empty())
		vecDrawData.clear();

	if (pop_deadzone_next_frame) {
		pop_deadzone_next_frame = false;
		bDeadZone = false;
	}
}

void CRender::RenderDrawData() {
	std::unique_lock<std::shared_mutex> lock(drawMutex);

	if (!renderInitialized)
		return;

	if (vecSafeDrawData.empty())
		return;

	device->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
	device->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	device->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
	device->GetVertexDeclaration(&vert_dec);
	device->GetVertexShader(&vert_shader);

	device->SetVertexShader(nullptr);
	device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	device->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	device->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	device->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

	if (sprite->Begin(D3DXSPRITE_ALPHABLEND) != D3D_OK)
		return;

	device->GetFVF(&oldFVF);
	device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	for (const auto& data : vecSafeDrawData) {
		if (!data.object.has_value())
			continue;

		switch (data.type)
		{
		case EDrawType::PRIMITIVE: {
			const auto& object = std::any_cast<primitive_command_t>(data.object);
			device->DrawPrimitiveUP(object.primitiveType, object.primitiveCount, object.pVertexData.data(), sizeof(Vertex));
			break;
		}
		case EDrawType::TEXT: {
			const auto& object = std::any_cast<text_command_t>(data.object);

			std::wstring wtext = utf8_convert(object.text);
			D3DXFont* font = object.font;

			const wchar_t* wide_text = wtext.c_str();

			if (object.outlined)
			{
				device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

				DWORD outlineColor = (object.color & 0xFF000000) | 0x000000; // Black outline color with the same alpha as the text color
				char dummy[4];
				*(DWORD*)dummy = outlineColor;
				dummy[0] *= 0.3f;
				outlineColor = *(DWORD*)dummy;

				RECT outlineRect = { object.pRect.left - 1, object.pRect.top - 1, object.pRect.right - 1, object.pRect.bottom - 1 };

				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left - 1, object.pRect.top, object.pRect.right - 1, object.pRect.bottom };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left - 1, object.pRect.top + 1, object.pRect.right - 1, object.pRect.bottom + 1 };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left, object.pRect.top - 1, object.pRect.right, object.pRect.bottom - 1 };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left, object.pRect.top + 1, object.pRect.right, object.pRect.bottom + 1 };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left + 1, object.pRect.top - 1, object.pRect.right + 1, object.pRect.bottom - 1 };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left + 1, object.pRect.top + 1, object.pRect.right + 1, object.pRect.bottom + 1 };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				outlineRect = { object.pRect.left + 1, object.pRect.top, object.pRect.right + 1, object.pRect.bottom };
				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);

				font->DrawTextW(sprite, wtext.c_str(), const_cast<LPRECT>(&object.pRect), object.clipType, object.color);

				device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			}
			else if (object.dropshadow) {
				device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

				DWORD outlineColor = (object.color & 0xFF000000) | 0x000000; // Black outline color with the same alpha as the text color
				char dummy[4];
				*(DWORD*)dummy = outlineColor;
				dummy[0] *= 0.5f;
				outlineColor = *(DWORD*)dummy;

				RECT outlineRect = { object.pRect.left + 1, object.pRect.top + 1, object.pRect.right + 1, object.pRect.bottom + 1 };

				font->DrawTextW(sprite, wtext.c_str(), &outlineRect, object.clipType, outlineColor);
				font->DrawTextW(sprite, wtext.c_str(), const_cast<LPRECT>(&object.pRect), object.clipType, object.color);

				device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			}
			else {
				font->DrawTextW(sprite, wtext.c_str(), const_cast<LPRECT>(&object.pRect), object.clipType, object.color);
			}

			sprite->Flush();
			device->SetTexture(0, 0);

			break;
		}
		case EDrawType::IMAGE: {
			const auto& object = std::any_cast<image_command_t>(data.object);
			sprite->Draw(object.texture, object.clip ? &object.pSrcRect : NULL, NULL, &object.pos, object.color);

			sprite->Flush();
			device->SetTexture(0, 0);

			break;
		}
		case EDrawType::CLIP: {
			const auto& object = std::any_cast<clip_command_t>(data.object);
			if (object.remove) {
				device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			}
			else {
				device->SetScissorRect(&object.rect);
				device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
			}
			break;
		}
		case EDrawType::SETANTIALIAS: {
			const auto& object = std::any_cast<bool>(data.object);
			device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, object);
			device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, object);
			break;
		}
		}
	};

	device->SetFVF(oldFVF);
	sprite->End();

	DirectXDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
	DirectXDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
	DirectXDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
	DirectXDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
	DirectXDevice->SetVertexDeclaration(vert_dec);
	DirectXDevice->SetVertexShader(vert_shader);
}

void CRender::EndFrame() {
	std::unique_lock<std::shared_mutex> lock(drawMutex);
	vecDrawData.swap(vecSafeDrawData);
}

void CRender::Reset() {
	if (sprite)
		sprite->OnResetDevice();

	for (auto font : loaded_fonts) {
		font->font->OnResetDevice();
		font->_ts_font->OnResetDevice();
	}
}

void CRender::SetAntiAliasing(bool value) {
	vecDrawData.emplace_back(DrawCommand_t(EDrawType::SETANTIALIAS, value));
}

void CRender::BoxFilled(const Vector2& start, const Vector2& end, Color color, int rounding) {
	auto raw = color.to_int32();

	if (rounding == 0) {
		std::vector<Vertex> vertex({
			{start.x, end.y, color}, {start.x, start.y, color}, {end.x, end.y, color}, {end.x, start.y, color}
		});

		vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLESTRIP, 2, vertex)));
	}
	else {
		std::vector<Vertex> vertex({
			{start.x, end.y - rounding, color},
			{start.x, start.y + rounding, color},
			{start.x + 0.133974f * rounding, start.y + 0.5f * rounding, color},
			{start.x + 0.292893f * rounding, start.y + 0.292893f * rounding, color},
			{start.x + 0.5f * rounding, start.y + 0.133974f * rounding, color},
			{start.x + rounding, start.y, color},
			{end.x - rounding, start.y, color},
			{end.x - 0.5f * rounding, start.y + 0.133974f * rounding, color},
			{end.x - 0.292893f * rounding, start.y + 0.292893f * rounding, color},
			{end.x - 0.133974f * rounding, start.y + 0.5f * rounding, color},
			{end.x, start.y + rounding, color},
			{end.x, end.y - rounding, color},
			{end.x - 0.133974f * rounding, end.y - 0.5f * rounding, color},
			{end.x - 0.292893f * rounding, end.y - 0.292893f * rounding, color},
			{end.x - 0.5f * rounding, end.y - 0.133974f * rounding, color},
			{end.x - rounding, end.y, color},
			{start.x + rounding, end.y, color},
			{start.x + 0.5f * rounding, end.y - 0.133974f * rounding, color},
			{start.x + 0.292893f * rounding, end.y - 0.292893f * rounding, color},
			{start.x + 0.133974f * rounding, end.y - 0.5f * rounding, color},
			{start.x, end.y - rounding, color},
		});

		vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, 19, vertex)));
	}
}

void CRender::Box(const Vector2& start, const Vector2& end, Color color, int rounding) {
	if (!rounding) {
		std::vector<Vertex> vertex({
			{start.x, end.y, color}, {start.x, start.y, color}, {end.x, start.y, color}, {end.x, end.y, color}
		});

		vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_LINESTRIP, 3, vertex)));

		return;
	}

	std::vector<Vertex> vertex({
		{start.x, end.y - rounding, color},
		{start.x, start.y + rounding, color},
		{start.x + 0.133974f * rounding, start.y + 0.5f * rounding, color},
		{start.x + 0.292893f * rounding, start.y + 0.292893f * rounding, color},
		{start.x + 0.5f * rounding, start.y + 0.133974f * rounding, color},
		{start.x + rounding, start.y, color},
		{end.x - rounding, start.y, color},
		{end.x - 0.5f * rounding, start.y + 0.133974f * rounding, color},
		{end.x - 0.292893f * rounding, start.y + 0.292893f * rounding, color},
		{end.x - 0.133974f * rounding, start.y + 0.5f * rounding, color},
		{end.x, start.y + rounding, color},
		{end.x, end.y - rounding, color},
		{end.x - 0.133974f * rounding, end.y - 0.5f * rounding, color},
		{end.x - 0.292893f * rounding, end.y - 0.292893f * rounding, color},
		{end.x - 0.5f * rounding, end.y - 0.133974f * rounding, color},
		{end.x - rounding, end.y, color},
		{start.x + rounding, end.y, color},
		{start.x + 0.5f * rounding, end.y - 0.133974f * rounding, color},
		{start.x + 0.292893f * rounding, end.y - 0.292893f * rounding, color},
		{start.x + 0.133974f * rounding, end.y - 0.5f * rounding, color},
		{start.x, end.y - rounding, color},
	});

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_LINESTRIP, 20, vertex)));
}

void CRender::GradientBox(const Vector2& start, const Vector2& end, Color t_l, Color t_r, Color b_l, Color b_r) {
	std::vector<Vertex> vertex({
		{start.x, start.y, t_l}, {end.x, start.y, t_r}, {end.x, end.y, b_r}, {start.x, end.y, b_l}
	});

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, 2, vertex)));
}

void CRender::Line(const Vector2& start, const Vector2& end, Color color) {
	std::vector<Vertex> vertex({
		{start.x, start.y, color}, {end.x, end.y, color}
	});

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_LINESTRIP, 1, vertex)));
}

void CRender::PolyLine(std::vector<Vector2> points, Color color) {
	const int size = points.size();
	std::vector<Vertex> vertex(size);

	for (int i = 0; i < size; i++) {
		vertex[i] = Vertex(points[i].x, points[i].y, color);
	}
	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_LINESTRIP, points.size() - 1, vertex)));
}

void CRender::PolyFilled(std::vector<Vector2> points, Color color) {
	const int size = points.size();
	std::vector<Vertex> vertex(size);

	for (int i = 0; i < size; i++) {
		vertex[i] = Vertex(points[i].x, points[i].y, color);
	}
	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, size - 2, vertex)));
}

void CRender::Circle(const Vector2& center, float radius, Color color, int segments, float start, float end) {
	if (segments == -1)
		segments = radius * 6;

	const int size = segments + 1;
	std::vector<Vertex> vertex(size);

	for (int i = 0; i < size; i++) {
		float ang = DEG2RAD(start) + i * (6.2831853f / segments);
		vertex[i] = Vertex(center.x + cos(ang) * radius, center.y + sin(ang) * radius, color);
	}

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_LINESTRIP, segments, vertex)));
}

void CRender::CircleFilled(const Vector2& center, float radius, Color color, int segments) {
	if (segments == -1)
		segments = radius * 6;

	std::vector<Vertex> vertex(segments + 1);
	vertex[0] = Vertex(center.x, center.y, color);

	for (int i = 0; i < segments; i++) {
		float ang = i * (6.2831853f / segments);
		vertex[i + 1] = Vertex(center.x + cos(ang) * radius, center.y + sin(ang) * radius, color);
	}

	vertex[segments] = Vertex(center.x + radius, center.y, color);

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, segments - 1, vertex)));
}

void CRender::Circle3D(const Vector& center, float radius, Color color, bool filled) {
	const int segments = radius * 6;
	std::vector<Vector2> points;

	for (float rot = 0.f; rot < 2.f * PI_F; rot += (2.f * PI_F) / segments) {
		Vector pos3d = center + Vector(cos(rot), sin(rot), 0) * radius;
		Vector2 pos = WorldToScreen(pos3d);

		if (pos.Invalid())
			return;

		points.emplace_back(Vector2(pos.x, pos.y));
	}

	Vector2 endPoint = WorldToScreen(center + Vector(1, 0, 0) * radius);

	if (!endPoint.Invalid())
		points.emplace_back(Vector2(endPoint.x, endPoint.y));

	PolyLine(points, color);
	color.a *= 0.3;
	PolyFilled(points, color);
}

void CRender::Circle3DGradient(const Vector& center, float radius, Color color, bool reverse) {
	const auto center_color = (reverse ? color.alpha_modulate(0) : color).d3d_color();
	const auto out_color = (reverse ? color : color.alpha_modulate(0)).d3d_color();

	const Vector2 center_scr = WorldToScreen(center);

	if (center_scr.Invalid())
		return;

	std::vector<Vertex> vertecies({ Vertex(center_scr.x, center_scr.y, center_color) });

	const int segments = radius * 6;
	const float segment_size = 2.f * PI_F / segments;

	for (float rot = 0; rot < 2.f * PI_F; rot += segment_size) {
		const Vector2 point = WorldToScreen(Vector(center.x + radius * std::cos(rot), center.y + radius * std::sin(rot), center.z));

		if (point.Invalid())
			return;

		vertecies.push_back(Vertex(point.x, point.y, out_color));
	}

	const Vector2 end_point = WorldToScreen(Vector(center.x + radius, center.y, center.z));
	vertecies.push_back(Vertex(end_point.x, end_point.y, out_color));

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, vertecies.size() - 2, vertecies)));
}

void CRender::GlowCircle(const Vector2& center, float radius, Color color) {
	const int segments = radius * 6;

	std::vector<Vertex> vertex(segments + 1);
	vertex[0] = Vertex(center.x, center.y, color);

	for (int i = 0; i < segments; i++) {
		float ang = i * (6.2831853f / segments);
		vertex[i + 1] = Vertex(center.x + cos(ang) * radius, center.y + sin(ang) * radius, color.alpha_modulate(0));
	}

	vertex[segments] = Vertex(center.x + radius, center.y, color.alpha_modulate(0));

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, segments - 1, vertex)));
}

void CRender::GlowCircle2(const Vector2& center, float radius, Color centerColor, Color edgeColor) {
	const int segments = radius * 6;

	std::vector<Vertex> vertex(segments + 1);
	vertex[0] = Vertex(center.x, center.y, centerColor);

	for (int i = 0; i < segments; i++) {
		float ang = i * (6.2831853f / segments);
		vertex[i + 1] = Vertex(center.x + cos(ang) * radius, center.y + sin(ang) * radius, edgeColor);
	}

	vertex[segments] = Vertex(center.x + radius, center.y, edgeColor);

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::PRIMITIVE, primitive_command_t(D3DPT_TRIANGLEFAN, segments - 1, vertex)));
}

void CRender::AddFontFromMemory(void* file, unsigned int size) {
	AddFontMemResourceEx(file, size, 0, &nFonts);
}

D3DXFont* CRender::LoadFont(const std::string& fontname, int size, int weight, int flags) {
	D3DXFont* font = new D3DXFont;

	D3DXCreateFontA(device, size, 0, weight, 0, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, flags, DEFAULT_PITCH | FF_DONTCARE, fontname.c_str(), &font->font);
	D3DXCreateFontA(device, size, 0, weight, 0, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, flags, DEFAULT_PITCH | FF_DONTCARE, fontname.c_str(), &font->_ts_font);

	loaded_fonts.emplace_back(font);

	return font;
}

Vector2 CRender::CalcTextSize(const std::string& text, D3DXFont* font) {
	std::wstring w_text = utf8_convert(text);
	return font->CalcTextSizeW(w_text.c_str());
}

void CRender::Text(const std::string& text, const Vector2& pos, Color color, D3DXFont* font, int flags) {
	RECT rect(pos.x, pos.y, pos.x, pos.y);

	if (clipping) {
		rect.right = clipRect.right;
		rect.bottom = clipRect.bottom;
	}

	DWORD clipType = clipping ? DT_LEFT : DT_NOCLIP;

	if (flags & TEXT_CENTERED) {
		Vector2 text_size = CalcTextSize(text, font);
		rect.left -= text_size.x / 2;
	}

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::TEXT, text_command_t{ text, font, rect, clipType, color.d3d_color(), bool(flags & TEXT_OUTLINED), bool(flags & TEXT_DROPSHADOW)}));
}

void CRender::Text(const std::string& text, int x, int y, Color color, D3DXFont* font, int flags) {
	RECT rect(x, y, x, y);

	if (clipping) {
		rect.right = clipRect.right;
		rect.bottom = clipRect.bottom;
	}

	DWORD clipType = clipping ? DT_LEFT : DT_NOCLIP;

	if (flags & TEXT_CENTERED) {
		Vector2 text_size = CalcTextSize(text, font);
		rect.left -= text_size.x / 2;
	}

	vecDrawData.emplace_back(DrawCommand_t(EDrawType::TEXT, text_command_t{ text, font, rect, clipType, color.d3d_color(), bool(flags & TEXT_OUTLINED), bool(flags & TEXT_DROPSHADOW) }));
}



IDirect3DTexture9* CRender::LoadImageFromMemory(void* data, int dataSize, const Vector2& size) {
	IDirect3DTexture9* dummy;
	D3DXCreateTextureFromFileInMemoryEx(device, data, dataSize, size.x, size.y, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0x00000000, NULL, NULL, &dummy);
	return dummy;
}

void CRender::Image(IDirect3DTexture9* image, const Vector2& pos, Color color) {
	D3DXVECTOR3 _pos(pos.x, pos.y, 0.0f);

	if (clipping) {
		RECT lol;
		lol.left = clipRect.left - _pos.x;
		lol.top = clipRect.top - _pos.y;
		lol.right = clipRect.right - _pos.x;
		lol.bottom = clipRect.bottom - _pos.y;
		vecDrawData.emplace_back(DrawCommand_t(EDrawType::IMAGE, image_command_t(image, lol, _pos, color.d3d_color(), true)));
	}
	else
		vecDrawData.emplace_back(DrawCommand_t(EDrawType::IMAGE, image_command_t(image, RECT(), _pos, color.d3d_color(), false)));
}

void CRender::PushClipRect(const Vector2& start, const Vector2& end) {
	RECT rect(start.x, start.y, end.x, end.y);
	clipRect = rect;
	clipping = true;
	vecDrawData.emplace_back(DrawCommand_t(EDrawType::CLIP, clip_command_t(rect, false)));
}

void CRender::PopClipRect() {
	clipping = false;
	vecDrawData.emplace_back(DrawCommand_t(EDrawType::CLIP, clip_command_t(RECT(), true)));
}

void CRender::UpdateViewMatrix(const ViewMatrix& vm) {
	view_matrix = vm;
}

Vector2 CRender::WorldToScreen(const Vector& pos) {
	Vec4 clipCoords;
	clipCoords.x = pos.x * view_matrix.matrix[0] + pos.y * view_matrix.matrix[1] + pos.z * view_matrix.matrix[2] + view_matrix.matrix[3];
	clipCoords.y = pos.x * view_matrix.matrix[4] + pos.y * view_matrix.matrix[5] + pos.z * view_matrix.matrix[6] + view_matrix.matrix[7];
	clipCoords.z = pos.x * view_matrix.matrix[8] + pos.y * view_matrix.matrix[9] + pos.z * view_matrix.matrix[10] + view_matrix.matrix[11];
	clipCoords.w = pos.x * view_matrix.matrix[12] + pos.y * view_matrix.matrix[13] + pos.z * view_matrix.matrix[14] + view_matrix.matrix[15];

	if (clipCoords.w < 0.1f)
		return { -1, -1 };

	Vector NDC;
	NDC.x = clipCoords.x / clipCoords.w;
	NDC.y = clipCoords.y / clipCoords.w;
	NDC.z = clipCoords.z / clipCoords.w;
	Vector2 res;
	res.x = (Cheat.ScreenSize.x / 2 * NDC.x) + (NDC.x + Cheat.ScreenSize.x / 2);
	res.y = -(Cheat.ScreenSize.y / 2 * NDC.y) + (NDC.y + Cheat.ScreenSize.y / 2);
	return res;
}

Vector2 CRender::GetMousePos() {
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(GetForegroundWindow(), &p);

	return Vector2((int)p.x, (int)p.y);
}

bool CRender::InBounds(Vector2 start, Vector2 end, bool ignore_deadzone) {
	Vector2 m = GetMousePos();

	if (!ignore_deadzone && bDeadZone && (m.x >= deadZone[0].x && m.y >= deadZone[0].y && m.x <= deadZone[1].x && m.y <= deadZone[1].y))
		return false;

	return m.x >= start.x && m.y >= start.y && m.x <= end.x && m.y <= end.y;
}

void CRender::PushDeadZone(Vector2 start, Vector2 end) {
	bDeadZone = true;
	deadZone[0] = start;
	deadZone[1] = end;
}

void CRender::PopDeadZone() {
	pop_deadzone_next_frame = true;
}

CRender* Render = new CRender;