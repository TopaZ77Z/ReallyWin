#pragma once
#include "../../SDK/Misc/Color.h"
#include "../../SDK/Interfaces/IMaterialSystem.h"

class IMaterial;

class CChams {
	bool IsLocalPlayerAttachment(CBaseEntity* entity);
public:
	void LoadChams();
	void ColorModulate(IMaterial* mat, Color color);
	void OverrideMaterial(int type, bool z, Color color, float glowThickness = 1);
	void OnDrawModelExecute(void* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* pBoneToWorld, void* edx);
};

extern CChams* Chams;