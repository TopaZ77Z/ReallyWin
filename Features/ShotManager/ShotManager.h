#pragma once

#include <vector>

#include "../../SDK/Misc/Vector.h"
#include "../../SDK/Misc/QAngle.h"


#define SHOOTPOS_BACKUP 90

class CBasePlayer;
class IGameEvent;
struct RegisteredShot_t;
struct ClientShot_t;

struct Impact_t {
	CBasePlayer* player;
	Vector position;
	int tick;
};

struct ClientShot_t {
	CBasePlayer*		player;
	Vector				position;
	QAngle				angle;
	int					tick;
	short				weapon_id;
	std::vector<Impact_t>impacts;
	RegisteredShot_t*	registered;
	bool				unregistered;
};

struct RegisteredShot_t {
	CBasePlayer*		player;
	Vector				position;
	QAngle				angle;
	int					tick;
	short				weapon_id;
	std::vector<Impact_t>impacts;
	ClientShot_t*		client;
	CBasePlayer*		hit_player;
};

class CShotManager {
	std::vector<ClientShot_t>		m_ClientShots;
	std::vector<RegisteredShot_t>	m_RegisteredShots;
	std::vector<Impact_t>			m_UnprocessedImpacts;

	Vector							m_ShootingPositions[64][SHOOTPOS_BACKUP];

public:
	void	OnCreateMove();
	void	OnNetUpdate();
	void	OnEvent(IGameEvent* event);
	void	Reset();
};