#pragma once

#include <glm/glm.hpp>

typedef struct Vector3 {
	float x;
	float y;
	float z;

	Vector3(float x, float y, float z) :
		x(x),
		y(y),
		z(z)
	{
	};
	Vector3(int x, int y, int z) :
		x(x),
		y(y),
		z(z)
	{
	};

} Vector3;

typedef struct CVector3 {
	GLubyte x;
	GLubyte y;
	GLubyte z;

	CVector3(GLubyte x, GLubyte y, GLubyte z) :
		x(x),
		y(y),
		z(z)
	{
	};
	CVector3(int x, int y, int z) :
		x(x),
		y(y),
		z(z)
	{
	};

} CVector3;