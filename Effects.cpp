#define _USE_MATH_DEFINES

#include <math.h>
#include <random>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "HelperGL/Shader.h"
#include "Constants.h"


unsigned starTex(NOT_INIT);
unsigned arrowTex(NOT_INIT);
unsigned bgPicture(NOT_INIT);

float curTime(NOT_INIT);
float deltaTime(NOT_INIT);

std::random_device rd;
std::mt19937 randGen(rd());

void drawQuad();
void drawDiscoCircle();
void drawAttractor(unsigned tempNum = MAX_LORENZ_SIZE);


void Background(Shader bgShader, glm::vec3 shakeOffset)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, BG_DISTANCE) + shakeOffset);
	model = glm::scale(model, glm::vec3(16.0f, 9.0f, 1.0f)); //Todo (to fix)

	bgShader.use();
	bgShader.setMat4("model", model);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bgPicture);
	drawQuad();
}


void StarField(Shader starShader, std::vector<Arrow>& starPos, float speedBoost)
{
	static std::uniform_real_distribution<> distXY(-3.0, 3.0);
	static std::uniform_real_distribution<> distZ(-80.0, 100.0);

	starShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, starTex);

	for (int i(0); i < starPos.size(); ++i)
	{
		if (starPos[i].pos.z >= 0)
			starPos[i].pos = { distXY(randGen), distXY(randGen), distZ(randGen) };
		else
			starPos[i].pos.z += 100 * speedBoost * deltaTime + 0.2f;

		glm::mat4 model = glm::translate(glm::mat4(1.0), starPos[i].pos);
		model = glm::scale(model, glm::vec3(0.01, 0.01, 1));
		starShader.setMat4("model", model);

		drawQuad();
	}
}


void LorenzAttractor(Shader lorenzShader, float fillIncrement)
{
	static float color(0);
	color += fillIncrement - deltaTime;

	lorenzShader.use();
	glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, -5));
	model = glm::scale(model, glm::vec3(0.03f));
	model = glm::rotate(model, glm::radians(20 * curTime), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-30.0, 0, 0));
	lorenzShader.setMat4("model", model);

	lorenzShader.setInt("colored", false);
	glLineWidth(1);
	drawAttractor();
	lorenzShader.setInt("colored", true);
	glLineWidth(3);
	drawAttractor(unsigned(10 * color));
}


void DiscoCircle(Shader circleShader, float freq[FREQ_NUMBER], glm::vec3 shakeOffset)
{
	circleShader.use();
	// Загружаем массив в шейдеры
	glUniform1fv(glGetUniformLocation(circleShader.ID, "scale"), 360, freq);

	glm::mat4 model = glm::translate(glm::mat4(1.0), shakeOffset);

	circleShader.setFloat("bassScale", 0.5f * freq[5] + 1.0f);
	circleShader.setMat4("model", model);

	glLineWidth(3);
	drawDiscoCircle();
}


void FlockingArrows(Shader arrowShader, std::vector<Arrow>& arrowPos, float freq[512], float colorBoost)
{
	static std::uniform_real_distribution<> dist(-3.0, 3.0);
	static unsigned modelLocation = glGetUniformLocation(arrowShader.ID, "model");
	static unsigned colorLocation = glGetUniformLocation(arrowShader.ID, "color");
	static unsigned bassScaleLocation = glGetUniformLocation(arrowShader.ID, "bassScale");

	if (arrowPos.size() < 300)
		for (int i(0); i < 15; ++i)
			if (freq[i * 6] > 0.01)
				arrowPos.push_back(Arrow{ glm::vec3(i / 2.0f - 3.5f, 2.0f, 0.0f), glm::vec2(dist(randGen), 3 + 2 * freq[5]),
					glm::vec2(0, 0), glm::vec3(1.0f - i / 16.0f, i / 16.0f, 8.0f - pow(i - 8, 2)) });

	arrowShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, arrowTex);

	for (int i(0); i < arrowPos.size(); ++i)
	{
		glm::vec2 avgVel(0);
		glm::vec3 avgPos(0), sepVec(0);
		float countOther(0);
		for (Arrow& other : arrowPos)
		{
			glm::vec2 between = arrowPos[i].pos - other.pos;
			if (between != glm::vec2(0.0) && between.y >= 0 && glm::length(between) < 1)
			{
				avgVel += other.vel;
				avgPos += other.pos;
				sepVec += glm::vec3(glm::normalize(between), 0.0);
				++countOther;
			}
		}

		if (countOther)
		{
			arrowPos[i].acc = avgVel / countOther - arrowPos[i].vel;
			arrowPos[i].acc += glm::vec2(50.0f * (avgPos / countOther - arrowPos[i].pos));
			arrowPos[i].acc += glm::vec2(sepVec / countOther - arrowPos[i].pos);
		}

		arrowPos[i].pos.x += arrowPos[i].vel.x * deltaTime;
		arrowPos[i].pos.y -= arrowPos[i].vel.y * deltaTime;
		arrowPos[i].vel.x += arrowPos[i].acc.x * deltaTime;
		arrowPos[i].vel.y -= arrowPos[i].acc.y * deltaTime;

		float angle = atan(arrowPos[i].vel.x / arrowPos[i].vel.y);

		if (abs(arrowPos[i].pos.y) > 2.0 || abs(arrowPos[i].pos.x) > 4.0)
		{
			arrowPos.erase(arrowPos.begin() + i);
			continue;
		}

		glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(arrowPos[i].pos.x, arrowPos[i].pos.y, -5));
		model = glm::rotate(model, angle, glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 1.0f));

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);
		glUniform3fv(colorLocation, 1, &arrowPos[i].col[0]);
		glUniform1f(bassScaleLocation, 1000 * colorBoost);

		drawQuad();
	}
}


//void MagicParticles(Shader particleShader, std::vector<Arrow>& particlePos, float freq[FREQ_NUMBER])
//{
//	static glm::vec3 up(0.0, 1.0, 0.0);
//	static glm::vec3 center(0.0, 0.0, 0.0);
//	static std::uniform_real_distribution<> distAngle(0.005f, 0.02f);
//
//	for (int i(0); i < 100; ++i)
//	{
//		glm::vec3 temp = 0.3f * glm::normalize(particlePos[i].pos);
//		temp = glm::rotateZ(temp, (float)distAngle(randGen));
//
//		if (particlePos[i].pos != glm::vec3(0))
//			particlePos[i].acc = 50.0f * glm::normalize(temp - particlePos[i].pos);
//		particlePos[i].vel += particlePos[i].acc * deltaTime;
//		particlePos[i].vel *= 0.97f;
//		particlePos[i].pos += glm::vec3(particlePos[i].vel, 0.0) * deltaTime;
//
//		for (int j(10); j < 180; ++j)
//			if (abs(acos(glm::dot(glm::normalize(particlePos[i].pos), up)) - j * M_PI / 180) < 5 * M_PI / 180 && glm::length(particlePos[i].pos) < 1.0f)
//				particlePos[i].vel += 4 * freq[j] * glm::vec2(particlePos[i].pos);
//
//		glm::mat4 model = glm::translate(glm::mat4(1.0), particlePos[i].pos);
//		model = glm::scale(model, glm::vec3(0.005f, 0.005f, 1.0f));
//
//		particleShader.use();
//		particleShader.setMat4("model", model);
//
//		drawQuad();
//	}
//}



// Disco circle
// -----------------------------------------
void drawDiscoCircle()
{
	static unsigned int circleVAO(0);
	static unsigned int circleVBO(0);

	if (!circleVAO)
	{
		constexpr glm::vec2 center{ 0, 0 };
		constexpr float radius = 0.3f;

		float vertices[720]{};
		for (int ii = 0; ii < 720; ii += 2)
		{
			float theta = 2.0f * (float)M_PI * float(ii) / float(360); // Get the current angle

			vertices[ii] = radius * sin(theta); // Calculate the x component
			vertices[ii + 1] = radius * cos(theta); // Calculate the y component
		}

		glGenVertexArrays(1, &circleVAO);
		glGenBuffers(1, &circleVBO);

		glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(circleVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	// Actual circle render
	glBindVertexArray(circleVAO);
	glDrawArrays(GL_LINE_LOOP, 0, 180);
	glBindVertexArray(0);
}


// Background
// -----------------------------------------
void drawQuad()
{
	static unsigned int quadVAO(0);
	static unsigned int quadVBO(0);

	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// координаты        // текстурные координаты
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// установка VAO плоскости
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


// Background
// -----------------------------------------
void drawAttractor(unsigned tempNum)
{
	static unsigned int lorenzVAO(0);
	static unsigned int lorenzVBO(0);

	const unsigned pointNum = 3 * MAX_LORENZ_SIZE;

	if (lorenzVAO == 0)
	{
		constexpr float a(20.0f), b(28.0f), c(8.0f / 3);
		float x(1.1f), y(0), z(1.0f);

		float* vertices = new float[pointNum] {};
		for (int i(0); i < pointNum; i += 3)
		{
			float dt = 0.01f;
			float dx = dt * (a * (y - x));
			float dy = dt * (x * (b - z) - y);
			float dz = dt * (x * y - c * z);
			x += dx; y += dy; z += dz;
			vertices[i] = x; vertices[i + 1] = y; vertices[i + 2] = z;
		}

		// установка VAO плоскости
		glGenVertexArrays(1, &lorenzVAO);
		glGenBuffers(1, &lorenzVBO);
		glBindVertexArray(lorenzVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lorenzVBO);
		glBufferData(GL_ARRAY_BUFFER, pointNum * sizeof(float), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}
	glBindVertexArray(lorenzVAO);
	glDrawArrays(GL_LINE_STRIP, 0, tempNum);
	glBindVertexArray(0);
}