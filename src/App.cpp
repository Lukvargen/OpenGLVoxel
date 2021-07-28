#include "App.h"





//void MakeQuad(std::vector<VertexData>& verts, std::vector<unsigned int>& indices, Vector3 a, Vector3 b, Vector3 c, Vector3 d, float light);
//void MakeCube(std::vector<VertexData>& verts, std::vector<unsigned int>& indices);


const unsigned int SCREEN_WIDTH = 1920;//800;
const unsigned int SCREEN_HEIGHT = 1080;//600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::ivec3 chunkPos = glm::vec3(-1000.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool wireframeMode = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float fov = 70.0f;


bool firstMouse = true;
float lastX = 400;
float lastY = 300;
float yaw = -90;
float pitch = 0;

//std::vector<Chunk*> chunks;
std::unordered_map<glm::ivec3, Chunk*> chunks; // all chunks, could set max loaded chunks. 
std::queue<Chunk*> chunksNeedGenerate;
std::vector<Chunk*> chunksNeedMeshing;

std::queue<Chunk*> chunksFree;
std::vector<Chunk*> chunksNeedRemove;



int main()
{


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "åpengeäl", NULL, NULL);
	
	if (window == NULL) 
	{
		std::cout << "Failed to create a window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// glad loads all opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to init GLAD" << std::endl;
		return -1;
	}
	// enable z buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// build and compile shader program
	// --------------------------------
	Shader shader("shaders/shader.vs", "shaders/shader.fs");
	

	// if draw in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); default
	

	glfwSwapInterval(0);
	// render loop
	float time = 0.0;
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		time += deltaTime;
		if (time >= 1.0) {
			std::cout << deltaTime << std::endl;
			time = 0;

		}

		// projection
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 300.0f);
		shader.setMat4("projection", projection);

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		shader.setMat4("view", view);

		// model
		//glm::mat4 model = chunk->transform;//glm::mat4(1.0f);
		//chunk->transform = glm::translate(chunk->transform, glm::vec3(0, sin((float)glfwGetTime())*0.2 , 0));
		//model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));

		shader.use();
		unsigned int modelLoc = glGetUniformLocation(shader.ID, "model");
		for (auto& c : chunks) {
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(c.second->transform));
			c.second->Draw();
		}
		
		

	

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	
	
	glfwTerminate();
	return 0;
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 20 * deltaTime;
	if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS))
		cameraSpeed *= 5;

	if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS))
		cameraPos += cameraSpeed * cameraFront;
	if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS))
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	
	glm::ivec3 currentChunkPos = glm::ivec3((int)cameraPos.x >> 5, (int)cameraPos.y >> 5, (int)cameraPos.z >> 5);
	if (chunkPos != currentChunkPos)
	{
		chunkPos = currentChunkPos;

		std::cout << " currentChunkPos " << currentChunkPos.x << " " << currentChunkPos.y << " " << currentChunkPos.z << std::endl;
		
		int viewDistance = 5;
		int deleteDistance = (viewDistance+1);// * Chunk::CHUNK_SIZE;

		
		// remove far away chunks
		std::vector<glm::ivec3> removeFromChunks;
		for (auto& c : chunks) 
		{
			Chunk* chunk = c.second;

			auto distVector = chunk->position - chunkPos;
			int dist = distVector.x * distVector.x + distVector.y * distVector.y + distVector.z * distVector.z;
			if (dist >= deleteDistance * deleteDistance)
			{
				chunksNeedRemove.push_back(chunk);
				removeFromChunks.push_back(chunk->position);
			}
		}
		for (auto key : removeFromChunks) // just to not add to remove list multiple times remve immidietly from chunks list
		{
			Chunk* c = chunks.find(key)->second;
			chunks.erase(key);
		}


		
		int chunkX;
		int chunkY;
		int chunkZ;
		for (int x = -viewDistance; x < viewDistance+1; x++)
		{
			chunkX = chunkPos.x + x;
			for (int y = -viewDistance; y < viewDistance+1; y++)
			{
				chunkY = chunkPos.y + y;
				for (int z = -viewDistance; z < viewDistance+1; z++)
				{
					chunkZ = chunkPos.z + z;

					auto distVector = glm::ivec3(chunkX, chunkY, chunkZ) - chunkPos;
					int dist = distVector.x * distVector.x + distVector.y * distVector.y + distVector.z * distVector.z;
					if (dist <= viewDistance * viewDistance)
					{
						auto findChunk = chunks.find(glm::ivec3(chunkX, chunkY, chunkZ));
						if (findChunk == chunks.end())
						{
							GenerateChunk(chunkX, chunkY, chunkZ);
						}
					}
					

					
				}
			}
		}
		

		
	}
	for (int i = chunksNeedRemove.size()-1; i >= 0; i--) // add to chunks free as soon as possible
	{
		Chunk* chunk = chunksNeedRemove.at(i);
		

		if (chunk->chunksUsingData != 0)
		{
			std::cout << "Chunk data occupied " << chunk->chunksUsingData << std::endl;
			continue;
		}
		if (chunk->locked)
		{
			std::cout << "LOCKED!" << std::endl; // meshing in progress
			continue;

		}

		glm::ivec3 cPos = chunk->position * Chunk::CHUNK_SIZE;
		chunk->erase();
		chunk->hide();
		chunksFree.push(chunk);
		chunksNeedRemove.erase(chunksNeedRemove.begin()+i);
	}


	int chunksGenerated = 0;
	while (!chunksNeedGenerate.empty())
	{
		Chunk* chunk = chunksNeedGenerate.front();
		glm::ivec3 cPos = chunk->position * Chunk::CHUNK_SIZE;
		chunk->generate(cPos.x, cPos.y, cPos.z);
		chunksNeedGenerate.pop();

		chunksGenerated++;
		if (chunksGenerated > 3)
			break;
	}

	int chunksMeshed = 0;
	while (!chunksNeedMeshing.empty())
	{
		int i = chunksNeedMeshing.size() - 1;
		Chunk* chunk = chunksNeedMeshing.at(i);
		glm::ivec3 cPos = chunk->position * Chunk::CHUNK_SIZE;
		chunk->mesh();
		//chunksNeedMeshing.pop();
		chunksNeedMeshing.erase(chunksNeedMeshing.begin()+i);

		chunksMeshed++;
		if (chunksMeshed > 3)
			break;
	}


}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		wireframeMode = !wireframeMode;
		if (wireframeMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;


	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void mouse_button_callback(GLFWwindow* window,int button, int action, int mods)
{
	if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
	{
		int x = (int)cameraPos.x;
		int y = (int)cameraPos.y;
		int z = (int)cameraPos.z;

		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
		
			//std::cout << "click " << cameraFront.x << " " << cameraFront.y << " " << cameraFront.z << std::endl;
		
			

			//std::cout << "is block solid " << isBlockSolid(x, y, z) << std::endl;

			placeBlock(BlockType::AIR,x, y, z);

			//isBlockSolid(x, y, z);
			//auto chunk = GetChunk()

		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			placeBlock(BlockType::GRASS, x, y, z);
		}
	}


}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset*5;
	//if (fov < 1.0f)
	//	fov = 1.0f;
	//if (fov > 45.0f)
	//	fov = 45.0f;
}
/*
void MakeQuad(std::vector<VertexData> &verts, std::vector<unsigned int> &indices, Vector3 a, Vector3 b, Vector3 c, Vector3 d, float light)
{
	unsigned int length = verts.size();
	indices.push_back(length);
	indices.push_back(length+1);
	indices.push_back(length+2);

	indices.push_back(length);
	indices.push_back(length+2);
	indices.push_back(length+3);
	// opengl likes to do things in reverse order
	verts.push_back(VertexData(d, light));
	verts.push_back(VertexData(c, light));
	verts.push_back(VertexData(b, light));
	verts.push_back(VertexData(a, light));
}*/
/*
void MakeCube(std::vector<VertexData>& verts, std::vector<unsigned int>& indices)
{
	// left
	MakeQuad(verts, indices, Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(0, 1, 1), Vector3(0, 0, 1), 0.7f);
	//right
	MakeQuad(verts, indices, Vector3(1, 0, 1), Vector3(1, 1, 1), Vector3(1, 1, 0), Vector3(1, 0, 0), 0.8f);
	//down
	MakeQuad(verts, indices, Vector3(1, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(1, 0, 1), 0.5f);
	//up
	MakeQuad(verts, indices, Vector3(1, 1, 1), Vector3(0, 1, 1), Vector3(0, 1, 0), Vector3(1, 1, 0), 1.0f);
	//back
	MakeQuad(verts, indices, Vector3(1, 0, 0), Vector3(1, 1, 0), Vector3(0, 1, 0), Vector3(0, 0, 0), 0.6f);
	//front
	MakeQuad(verts, indices, Vector3(0, 0, 1), Vector3(0, 1, 1), Vector3(1, 1, 1), Vector3(1, 0, 1), 0.9f);
	

}*/

void GenerateChunk(int x, int y, int z)
{
	//std::cout << "Generate Chunk Time" << std::endl;
	//Timer time("Cunk Time");

	glm::mat4 transform = glm::mat4(1.0f);
	glm::vec3 pos = glm::ivec3(x, y, z);
	transform = glm::translate(transform, pos*(float)Chunk::CHUNK_SIZE);


	Chunk* chunk;
	if (!chunksFree.empty())
	{
		//std::cout << "chunks free " << chunksFree.size() << std::endl;
		chunk = chunksFree.front();
		chunksFree.pop();
		chunk->transform = transform;
		chunk->position = pos;
		chunk->reset();
		
	}
	else {
		chunk = new Chunk(transform, pos);
		chunk->reset();
		//std::cout << "No free chunks" << chunksFree.size() << std::endl;
		

	}
	chunks.insert(std::pair<glm::ivec3, Chunk*>(pos, chunk));
	
	
	chunksNeedGenerate.push(chunk);
}



Chunk* GetChunk(glm::ivec3 pos)
{
	auto chunk = chunks.find(pos);
	if (chunk == chunks.end())
		return nullptr;
	return chunk->second;
}
Chunk* GetChunk(int x, int y, int z)
{
	auto chunk = chunks.find(glm::ivec3(x,y,z));
	if (chunk == chunks.end())
		return nullptr;
	return chunk->second;
}

Chunk* GetOrCreateChunk(int x, int y, int z)
{
	auto chunk = chunks.find(glm::ivec3(x,y,z));
	if (chunk == chunks.end())
		GenerateChunk(x, y, z);
		return nullptr;
	return chunk->second;
}

void AddToMeshQueue(Chunk* chunk)
{
	chunksNeedMeshing.push_back(chunk);
}

bool isBlockSolid(int x, int y, int z)
{
	int chunkSize = Chunk::CHUNK_SIZE;
	int i = log2(chunkSize);
	int chunkX = x >> i;
	int chunkY = y >> i;
	int chunkZ = z >> i;
	auto chunk = GetChunk(chunkX, chunkY, chunkZ);
	if (chunk != nullptr)
	{
		int blockX = x - chunkX * chunkSize;
		int blockY = y - chunkY * chunkSize;
		int blockZ = z - chunkZ * chunkSize;

		std::cout << "get block pos " << blockX << " " << blockY << " " << blockZ << std::endl;
		if (chunk->generated)
		{
			return chunk->IsSolid(blockX, blockY, blockZ);
		}
	}

	return false;
}

void placeBlock(BlockType block,int x, int y, int z)
{
	int chunkSize = Chunk::CHUNK_SIZE;
	int i = log2(chunkSize);
	int chunkX = x >> i;
	int chunkY = y >> i;
	int chunkZ = z >> i;
	auto chunk = GetChunk(chunkX, chunkY, chunkZ);
	if (chunk != nullptr)
	{
		int blockX = x - chunkX * chunkSize;
		int blockY = y - chunkY * chunkSize;
		int blockZ = z - chunkZ * chunkSize;

		chunk->setBlock(block, blockX, blockY, blockZ);
		/*
		for (auto c : chunksNeedMeshing)
		{
			if (chunk == c)
			{
				std::cout << "works" << std::endl;
				return;
			}
		}
		AddToMeshQueue(chunk);*/
		chunk->visible = false;
		chunk->updateNeighbours(); // change name...
		//chunk->mesh();
		//chunk->visible = false;
	}

	

}

