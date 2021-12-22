#pragma once

#define NOT_INIT -1
#define FREQ_NUMBER 512
#define FREQ_BASS_MODE DWORD(BASS_DATA_FFT256 + log2(FREQ_NUMBER) - 7)
#define MAX_LORENZ_SIZE 2000
#define BG_DISTANCE -20
//#define CIRCLE_VERTICES 


enum class Scene
{
	BACKGROUND = 1,
	STAR_FIELD = 2,
	FLOCKINT_ARROWS = 4,
	LORENZ_ATTRACTOR = 8,
	DISCO_CIRCLE = 16,
	MAGIC_PARTICLES = 32
};

struct Arrow
{
	glm::vec3 pos{ 0, 0, 0 };
	glm::vec2 vel{ 0, 0 };
	glm::vec2 acc{ 0, 0 };
	glm::vec3 col{ 0, 0, 0 };
};