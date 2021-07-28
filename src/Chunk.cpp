#include "Chunk.h"


void Chunk::generate(int chunkX, int chunkY, int chunkZ)
{

	// todo
	/*
	ambient occlusion ++
	multithreading
	better world gen
	collisions
	walkable player
	debug see chunk borders
	raycast
	

	*/

	locked = true;
	

	//verts.clear();
	//indices.clear();

	//siv::PerlinNoise noise(12345);
	//OSN::Noise<2> noiseTest;
	
	FastNoiseLite noise(0);
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	//noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
	//noise.SetFractalOctaves(4);
	//noise.SetFrequency(0.005);
	//noise.SetFractalGain(0.5f);
	//noise.SetFractalWeightedStrength(0.5f);
	


	int heightVariance = 16;

	for (int x = 0; x < CHUNK_SIZE; x++) {
		int globalX = chunkX + x;
			for (int z = 0; z < CHUNK_SIZE; z++) {
				int globalZ = chunkZ + z;
				int height = noise.GetNoise((float)globalX, (float)globalZ) * heightVariance + heightVariance;
				
				for (int y = 0; y < CHUNK_SIZE; y++) {
					int globalY = chunkY + y;

					if (height > globalY)
						blocks[x][y][z] = BlockType::GRASS;
					else
						blocks[x][y][z] = BlockType::AIR;

					if (noise.GetNoise((float)globalX, (float)globalY, (float)globalZ) > 0)
						blocks[x][y][z] = BlockType::GRASS;

				}
			}
	}

	//mesh();

	generateComplete();

	
}


void Chunk::generateComplete()
{
	generated = true;
	locked = false;

	int x = position.x;
	int y = position.y;
	int z = position.z;

	f = GetChunk(x, y, z + 1);
	b = GetChunk(x, y, z - 1);
	u = GetChunk(x, y + 1, z);
	d = GetChunk(x, y - 1, z);
	r = GetChunk(x + 1, y, z);
	l = GetChunk(x - 1, y, z);

	if (f)
		f->updateNeighbours();
	if (b)
		b->updateNeighbours();
	if (u)
		u->updateNeighbours();
	if (d)
		d->updateNeighbours();
	if (r)
		r->updateNeighbours();
	if (l)
		l->updateNeighbours();

	updateNeighbours();

	
}

void Chunk::updateNeighbours()
{
	if (!generated || visible)
		return;

	int x = position.x;
	int y = position.y;
	int z = position.z;

	f = GetChunk(x, y, z + 1);
	b = GetChunk(x, y, z - 1);
	u = GetChunk(x, y + 1, z);
	d = GetChunk(x, y - 1, z);
	r = GetChunk(x + 1, y, z);
	l = GetChunk(x - 1, y, z);

	if (f && b && u && d && r && l)
	{
		if (f->generated && b->generated && u->generated && d->generated && r->generated && l->generated)
		{
			f->chunksUsingData++;
			b->chunksUsingData++;
			u->chunksUsingData++;
			d->chunksUsingData++;
			r->chunksUsingData++;
			l->chunksUsingData++; 

			AddToMeshQueue(this);
			locked = true;
		}
	}
}

void Chunk::mesh()

{
	locked = true;
	std::vector<VertexData> verts;
	std::vector<unsigned int> indices;

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				if (blocks[x][y][z] == BlockType::AIR) {
					continue;
				}

				float randLight = (rand() / (RAND_MAX + 1.0f) * 0.05);

				// left
				if (!IsSolid(x - 1, y, z))
					MakeQuad(verts, indices, Vector3(x, y, z), Vector3(x, y + 1, z), Vector3(x, y + 1, z + 1), Vector3(x, y, z + 1), 0.7f + randLight);

				//right
				if (!IsSolid(x + 1, y, z))
					MakeQuad(verts, indices, Vector3(x + 1, y, z + 1), Vector3(x + 1, y + 1, z + 1), Vector3(x + 1, y + 1, z), Vector3(x + 1, y, z), 0.8f + randLight);
				//down
				if (!IsSolid(x, y - 1, z))
					MakeQuad(verts, indices, Vector3(x + 1, y, z), Vector3(x, y, z), Vector3(x, y, z + 1), Vector3(x + 1, y, z + 1), 0.5f + randLight);
				//up
				if (!IsSolid(x, y + 1, z))
					MakeQuad(verts, indices, Vector3(x + 1, y + 1, z + 1), Vector3(x, y + 1, z + 1), Vector3(x, y + 1, z), Vector3(x + 1, y + 1, z), 1.0f + randLight);
				//back
				if (!IsSolid(x, y, z - 1))
					MakeQuad(verts, indices, Vector3(x + 1, y, z), Vector3(x + 1, y + 1, z), Vector3(x, y + 1, z), Vector3(x, y, z), 0.6f + randLight);
				//front
				if (!IsSolid(x, y, z + 1))
					MakeQuad(verts, indices, Vector3(x, y, z + 1), Vector3(x, y + 1, z + 1), Vector3(x + 1, y + 1, z + 1), Vector3(x + 1, y, z + 1), 0.9f + randLight);
			}
		}
	}



	indicesSize = indices.size();

	if (!indices.empty())
	{
		glBindVertexArray(VAO);


		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * verts.size(), &verts[0], GL_STATIC_DRAW);



		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0); // funkar inte med 3 byte pga padding?
		glEnableVertexAttribArray(0);
		// light
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

	}
	meshComplete();
		

}

void Chunk::meshComplete()
{
	f->chunksUsingData--;
	b->chunksUsingData--;
	u->chunksUsingData--;
	d->chunksUsingData--;
	r->chunksUsingData--;
	l->chunksUsingData--;
	locked = false;
	visible = true;

}

void Chunk::erase() 
{
	if (f)
		f->updateNeighbours();
	if (b)
		b->updateNeighbours();
	if (u)
		u->updateNeighbours();
	if (d)
		d->updateNeighbours();
	if (r)
		r->updateNeighbours();
	if (l)
		l->updateNeighbours();
}

void Chunk::reset()
{
	visible = false;
	generated = false;
	locked = false;
}

void Chunk::setBlock(BlockType block, int x, int y, int z)
{
	blocks[x][y][z] = block;

}

void Chunk::MakeQuad(std::vector<VertexData> &verts, std::vector<unsigned int> &indices, Vector3 a, Vector3 b, Vector3 c, Vector3 d, float light)
{
	unsigned int length = verts.size();
	indices.push_back(length);
	indices.push_back(length + 1);
	indices.push_back(length + 2);

	indices.push_back(length);
	indices.push_back(length + 2);
	indices.push_back(length + 3);



	verts.push_back(VertexData(d, light));
	verts.push_back(VertexData(c, light));
	verts.push_back(VertexData(b, light));
	verts.push_back(VertexData(a, light));

}

inline int calculateVertexAO(const Vector3& indexPos)
{

}

void Chunk::Draw() 
{
	if (!visible)
		return;
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
}

bool Chunk::IsSolid(int x, int y, int z)
{
	//if (x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE || x < 0 || y < 0 || z < 0)
	//	return false;
	if (x == -1)
		return l->IsSolid(CHUNK_SIZE -1, y, z);
	else if (x == CHUNK_SIZE)
		return r->IsSolid(0, y, z);
	else if (y == -1)
		return d->IsSolid(x, CHUNK_SIZE-1, z);
	else if (y == CHUNK_SIZE)
		return u->IsSolid(x, 0, z);
	else if (z == -1)
		return b->IsSolid(x, y, CHUNK_SIZE - 1);
	else if (z == CHUNK_SIZE)
		return f->IsSolid(x, y, 0);


	return blocks[x][y][z] != BlockType::AIR;
}

void Chunk::hide()
{
	
	visible = false;
}

Chunk* Chunk::GetAndNotify(int x, int y, int z)
{
	Chunk* c = GetChunk(x, y, z);
	if (c)
	{
		
	}
}