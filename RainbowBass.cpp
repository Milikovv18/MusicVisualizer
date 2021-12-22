// RainbowBass.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <numeric>
#include <random>

#include <bass.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Constants.h"
#include "HelperGL/Shader.h"
#include "HelperGL/Texture.h"


constexpr const char* textureDir = "./Textures"; // Путь ко всем текстурам
unsigned int MASK = 0b010011;


extern float curTime;
extern float deltaTime;
extern unsigned arrowTex;
extern unsigned starTex;
extern unsigned bgPicture;


// Helper functions
void processInputs(GLFWwindow* window, int key, int scancode, int action, int mods);
void processFFT(HCHANNEL channel, float freq[FREQ_NUMBER], float freqDiff[FREQ_NUMBER], float freqPerc[FREQ_NUMBER]);

// Geometry functions
void StarField(Shader starShader, std::vector<Arrow>& starPos, float speedBoost);
void LorenzAttractor(Shader lorenzShader, float fillIncrement);
void DiscoCircle(Shader circleShader, float freq[FREQ_NUMBER], glm::vec3 shakeOffset);
void FlockingArrows(Shader arrowShader, std::vector<Arrow>& arrowPos, float freq[512], float colorBoost);
void Background(Shader bgShader, glm::vec3 shakeOffset);
void MagicParticles(Shader particleShader, std::vector<Arrow>& particlePos, float freq[FREQ_NUMBER]);


int main()
{
	// Bass initialization
    BASS_Init(-1, 44100, 0, 0, NULL);
    HMUSIC sample = BASS_SampleLoad(false, "Music/Phoenix.mp3", 0, 0, 1, BASS_SAMPLE_MONO);
    HCHANNEL channel = BASS_SampleGetChannel(sample, FALSE);
    BASS_ChannelPlay(channel, FALSE);

	// Window creation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// RUNTIME VARIABLES //
	#pragma region RUNTIME_VARIABLES
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	const unsigned SCREEN_WIDTH = desktop.right;
	const unsigned SCREEN_HEIGHT = desktop.bottom;
	#pragma endregion

	// Creating output window
#ifdef _DEBUG
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Moosic visualizer", nullptr, nullptr);
#else
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Moosic visualizer", glfwGetPrimaryMonitor(), nullptr);
#endif

	if (!window)
	{
		std::cout << "Window didnt created" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, processInputs);

	// Setting up GLAD
	int err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!err)
	{
		std::cout << "GLAD didnt go right" << std::endl;
		return -2;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Presettings
	glEnable(GL_DEPTH_TEST);


	// ACTUAL PROGRAM *//
	// Shaders
	Shader circle("Shaders/Circle.vert", "Shaders/Circle.frag");
	Shader background("Shaders/Background.vert", "Shaders/Background.frag");
	Shader flocking("Shaders/Flocking.vert", "Shaders/Flocking.frag");
	Shader lorenz("Shaders/Lorenz.vert", "Shaders/Lorenz.frag");
	Shader particle("Shaders/Particle.vert", "Shaders/Particle.frag");

	glm::mat4 model(1.0);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.1f);

	circle.use();
	circle.setMat4("projection", projection);

	background.use();
	background.setInt("picture", 0);
	background.setMat4("projection", projection);

	flocking.use();
	flocking.setInt("arrowTex", 0);
	flocking.setMat4("projection", projection);

	lorenz.use();
	lorenz.setMat4("projection", projection);

	particle.use();
	particle.setMat4("projection", projection);

	// Textures
	// Backgound: Unsplash Sky:24 Daniel Leone https://unsplash.com/photos/v7daTKlZzaw #230
	bgPicture = TextureFromFile("sunrise.jpg", textureDir, false);
	arrowTex = TextureFromFile("arrow.png", textureDir, false);
	starTex = TextureFromFile("star.png", textureDir, false);

	// Prerender
	std::vector<Arrow> arrowPos;
	arrowPos.reserve(100);
	arrowPos.resize(100);

	float freq[FREQ_NUMBER]{}, freqDiff[FREQ_NUMBER]{}, freqPerc[FREQ_NUMBER]{};
	float shaking = 0.0f, color = 0.0f;
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


	// todo temp
	for (int i(0); i < 100; ++i)
		arrowPos[i].vel = glm::vec2(rand() % 5 + 1, rand() % 5 + 1);


	// RENDER //
	float lastTime(0);
	while (!glfwWindowShouldClose(window))
	{
		curTime = (float)glfwGetTime();
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		processFFT(channel, freq, freqDiff, freqPerc);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Setting up shaking
		shaking = std::max(freq[5] * 0.01f, shaking - 0.05f);
		glm::vec3 shakeOffset(sin(40 * glfwGetTime()), sin(35 * (glfwGetTime() + 5)), 0);
		shakeOffset *= 10 * shaking;


		// Background
		if (MASK & unsigned(Scene::BACKGROUND))
			Background(background, shakeOffset);


		if (MASK & unsigned(Scene::STAR_FIELD))
		{
			// Star field
			if (MASK & unsigned(Scene::FLOCKINT_ARROWS))
				FlockingArrows(flocking, arrowPos, freq, shaking);

			// Flocking
			else
				StarField(background, arrowPos, freq[5]);
		}


		// Lorenz Attractor
		if (MASK & unsigned(Scene::LORENZ_ATTRACTOR))
			LorenzAttractor(lorenz, freq[5]);


		// Disco circle
		if (MASK & unsigned(Scene::DISCO_CIRCLE))
			DiscoCircle(circle, freq, shakeOffset);


		if (MASK & unsigned(Scene::MAGIC_PARTICLES))
			MagicParticles(particle, arrowPos, freqDiff);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}


// FFT analysis
void processFFT(HCHANNEL channel, float freq[FREQ_NUMBER], float freqDiff[FREQ_NUMBER], float freqPerc[FREQ_NUMBER])
{
	float freqSum(0);
	freqSum = std::accumulate(freq, freq + FREQ_NUMBER, 0.0f);
	memcpy_s(freqDiff, FREQ_NUMBER, freq, FREQ_NUMBER);

	static float fft[FREQ_NUMBER];
	BASS_ChannelGetData(channel, fft, FREQ_BASS_MODE);
	for (int i(0); i < FREQ_NUMBER; ++i)
	{
		freq[i] = (freq[i] + fft[i]) / 2; // Filling freqs
		freqDiff[i] = freq[i] - freqDiff[i]; // Filling freqDiff
		freqPerc[i] = 100 * freq[i] / freqSum; // Filling freqPerc
	}
}


// Key press handler
void processInputs(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	for (int i(0); i < 6; ++i)
		if ((key == GLFW_KEY_1 + i) && action == GLFW_PRESS)
			MASK ^= 1 << i;
}