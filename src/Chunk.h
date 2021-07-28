#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include "VertexData.h"
#include "FastNoiseLite.h"

#include <random>


#include "Block.h"

#include <iostream>
#include "ProfileTimer.h"




class Chunk
{

public:
	glm::mat4 transform;
	glm::ivec3 position;
	static const int CHUNK_SIZE = 32;
	bool visible = false;
	bool generated = false;
	int chunksUsingData = 0;
	bool locked = false;

	

	Chunk(glm::mat4 transform, glm::ivec3 position):
		transform(transform), position(position)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
	}

	void init(Chunk* f, Chunk* b, Chunk* u, Chunk* d, Chunk* r, Chunk* l);
	void generate(int chunkX, int chunkY, int chunkZ);
	
	void mesh();
	void updateNeighbours();

	void hide();
	bool IsSolid(int x, int y, int z);

	void erase();
	void reset();

	void setBlock(BlockType block ,int x, int y, int z);

	



	void Draw();

private:
	unsigned int VBO, VAO, EBO;
	//std::vector<VertexData> verts;
	//std::vector<unsigned int> indices;
	int indicesSize = 0;
	BlockType blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	Chunk* f;
	Chunk* b;
	Chunk* u;
	Chunk* d;
	Chunk* r;
	Chunk* l;
	


	void MakeCube(int x, int y, int z);
	void MakeQuad(std::vector<VertexData>& verts, std::vector<unsigned int>& indices, Vector3 a, Vector3 b, Vector3 c, Vector3 d, float light);
	Chunk* GetAndNotify(int x, int y, int z);
	void generateComplete();
	void meshComplete();
	




};

Chunk* GetChunk(int x, int y, int z);
Chunk* GetChunk(glm::ivec3 pos); // forward decloration, include app.h doesnt work idk how why loop thing. går nog om man har include efter class men men
Chunk* GetOrCreateChunk(int x, int y, int z);
void AddToMeshQueue(Chunk* chunk);