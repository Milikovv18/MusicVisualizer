#define _USE_MATH_DEFINES

#include <math.h>
#include <random>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "HelperGL/Shader.h"
#include "Constants.h"

extern float curTime;
extern float deltaTime;

static std::random_device rd;
static std::mt19937 randGen(rd());

void drawQuad();
void drawDiscoCircle();
void drawAttractor(unsigned tempNum = MAX_LORENZ_SIZE);


void MagicParticles(Shader particleShader, std::vector<Arrow>& particlePos, float freq[FREQ_NUMBER])
{
	static std::uniform_real_distribution<> distY(-0.5f, 0.5f);
	static glm::vec3 center(0.0, 0.0, 0.0), nextCenter = center;
	static float centerTime(1.0);

	nextCenter = glm::vec3((std::distance(freq, std::max_element(freq, freq + FREQ_NUMBER))) / 100.0 - 1.0, distY(randGen), 0.0);
	if (centerTime < 0 && center.x != nextCenter.x)
	{
		center = nextCenter;
		centerTime = 1.0f;
	}

	centerTime -= deltaTime;


	for (int i(0); i < 100; ++i)
	{
		if (particlePos[i].pos != glm::vec3(0))
			particlePos[i].acc = 300.0f * glm::normalize(center - particlePos[i].pos);
		particlePos[i].vel += particlePos[i].acc * deltaTime;
		particlePos[i].vel *= 0.9996f;
		particlePos[i].pos += glm::vec3(particlePos[i].vel, 0.0) * deltaTime;

		glm::mat4 model = glm::translate(glm::mat4(1.0), particlePos[i].pos);
		model = glm::scale(model, glm::vec3(0.005f, 0.005f, 1.0f));

		particleShader.use();
		particleShader.setMat4("model", model);

		drawQuad();
	}
}