#pragma once

#include "../../definitions.h"

// Implements the PS1 GTE (geometry transformation engine)
// which does all the 3D computations (the gpu only handles 2D drawing)
class GTE {
public:
	u32 data[32];
	u32 ctrl[32];


};