#pragma once


#include <glm/glm.hpp>
#include "Chunk.h"

class RayCast
{
public:
	glm::vec3 hitPos;
	glm::vec3 hitNormal;

	RayCast(glm::vec3 from, glm::vec3 to): hitPos(glm::vec3(0.0f)), hitNormal(glm::vec3(0.0f))
	{
		glm::vec3 dir = glm::normalize(to - from);

	}
};


bool isBlockSolid(int x, int y, int z);