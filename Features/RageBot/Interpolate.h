#pragma once

#include "../../SDK/Globals.h"
#include "../../SDK/Interfaces.h"

class CInterpolate
{
	struct interpolate_data_t {
		Vector origin;
		matrix3x4_t original_matrix[MAXSTUDIOBONES];
		bool valid = false;
	};

public:
	void	DisableInterpolationFlags(CBasePlayer* player);
	Vector	GetInterpolated(CBasePlayer* player);
	void	RunInterpolation();
	void	InterpolateModel(CBasePlayer* player, matrix3x4_t* matrix);
	void	ResetInterpolation();
	void	InvalidateInterpolation(int i);
	interpolate_data_t interpolate_data[64];
};

extern CInterpolate* Interpolate;
