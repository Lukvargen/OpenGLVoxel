#pragma once

#include "Vector3.h"

typedef struct VertexData {
	Vector3 pos;
	float lightMultiplier;
	//float ao;

	VertexData(Vector3 pos, float lightMultiplier) :
		pos(pos),
		lightMultiplier(lightMultiplier)
		//ao(ao)
	{

	};

} VertexData;