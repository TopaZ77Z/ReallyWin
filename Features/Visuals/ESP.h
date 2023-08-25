#pragma once
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBasePlayer.h"

struct ESPInfo_t {
	CBasePlayer*	m_pEnt = nullptr;
	Vector			m_vecOrigin;
	Vector2			m_BoundingBox[2];
	int				m_nFakeDuckTicks = 0;
	bool			m_bDormant = false;
	bool			m_bValid = false;
	int				m_Alpha = 0.f;
	float			m_flLastUpdateTime = 0.f;
	int             m_nFlags = 0;
};

struct ESPFlag_t {
	std::string flag;
	Color color;
};

extern ESPInfo_t ESPInfo[64];

namespace ESP {
	void		ProcessSounds();

	void		UpdatePlayer(int id);
	void		Draw();
	void		DrawPlayer(int id);
	void		DrawBox(ESPInfo_t info);
	void		DrawHealth(ESPInfo_t info);
	void		DrawName(ESPInfo_t info);
	void		DrawFlags(ESPInfo_t info);
	void		DrawWeapon(ESPInfo_t info);
	void		DrawSkeleton(ESPInfo_t info);

	void		DrawGrenades();
}