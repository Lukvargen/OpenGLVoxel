#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL // eller göra egna hash hur man nu gör de
#include "glm/gtx/hash.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>

#include "Chunk.h"

#include "ProfileTimer.h"


//class Chunk;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void GenerateChunk(int x, int y, int z);
void AddToMeshQueue(Chunk* chunk);

Chunk* GetChunk(int x, int y, int z);
Chunk* GetChunk(glm::ivec3 pos);
bool isBlockSolid(int x, int y, int z);
void placeBlock(BlockType block, int x, int y, int z);

Chunk* GetOrCreateChunk(int x, int y, int z);
