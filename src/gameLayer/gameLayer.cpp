#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "rendering/camera.h"
#include "errorReporting.h"
#include "rendering/renderer.h"
#include "chunkSystem.h"
#include "threadStuff.h"
#include <thread>
#include <ctime>
#include "multyPlayer/server.h"
#include "multyPlayer/createConnection.h"
#include <enet/enet.h>
#include "rendering/Ui.h"
#include "glui/glui.h"
#include "gamePlayLogic.h"
#include "multyPlayer/splitUpdatesLogic.h"
#include <rendering/renderSettings.h>

ProgramData programData;


bool initGame()
{

	createErrorFile();
	

	//gl2d::setVsync(false);
	programData.renderer2d.create();
	programData.font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	programData.uiTexture.loadFromFile(RESOURCES_PATH "ui0.png", true, true);
	programData.buttonTexture.loadFromFile(RESOURCES_PATH "beacon_button_default.png", true, true);
	programData.numbersTexture.loadFromFile(RESOURCES_PATH "numbers.png", true, true);
	//programData.dudv.loadFromFile(RESOURCES_PATH "otherTextures/test.jpg", true, true);
	programData.dudv.loadFromFile(RESOURCES_PATH "otherTextures/waterDUDV.png", false, true);
	//programData.dudv.loadFromFile(RESOURCES_PATH "otherTextures/wdudv.jpg", false, true);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	programData.causticsTexture.loadFromFile(RESOURCES_PATH "otherTextures/caustics.jpg", false, true);
	//programData.causticsTexture.loadFromFile(RESOURCES_PATH "otherTextures/caustics3.png", false, true);
	//programData.causticsTexture.loadFromFile(RESOURCES_PATH "otherTextures/test.jpg", false, true);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//programData.dudvNormal.loadFromFile(RESOURCES_PATH "otherTextures/normal.png", false, true);
	//programData.dudvNormal.loadFromFile(RESOURCES_PATH "otherTextures/normal2.png", false, true);
	//programData.dudvNormal.loadFromFile(RESOURCES_PATH "otherTextures/normal.jpg", false, true);
	//programData.dudvNormal.loadFromFile(RESOURCES_PATH "otherTextures/normal2.jpg", false, true);
	programData.dudvNormal.loadFromFile(RESOURCES_PATH "otherTextures/normal3.png", false, true); //best
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	programData.blocksLoader.loadAllTextures();
	programData.renderer.create(programData.blocksLoader);
	programData.gyzmosRenderer.create();
	programData.pointDebugRenderer.create();
	programData.modelsManager.loadAllModels();


	if (enet_initialize() != 0)
	{
		reportError("problem starting ENET");
		return false;
	}

	
	//programData.facesCount = blockData.size() / 4;

	//glNamedBufferData(programData.renderer.vertexBuffer, 0, 0, GL_DYNAMIC_DRAW);

	//glEnable(GL_LINE_WIDTH);
	glLineWidth(4);
	

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getWindowSizeX();
	h = platform::getWindowSizeY();

	programData.renderer2d.updateWindowMetrics(w, h);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#pragma endregion

#pragma region fps
	static float timeCounter = 0;
	static int frameCounter = 0;

	timeCounter += deltaTime;
	if(timeCounter >= 1.f)
	{
		timeCounter -= 1;
		programData.currentFps = frameCounter;
		frameCounter = 0;
	}
	frameCounter++;

#pragma endregion
	

	if (platform::isKeyPressedOn(platform::Button::P))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}

	static bool gameStarted = 0;
	static std::string lastError = "";
	static char ipString[50] = {};

	if (!gameStarted)
	{
		programData.menuRenderer.Begin(1);
		programData.menuRenderer.SetAlignModeFixedSizeWidgets({0,150});

		if (programData.menuRenderer.Button("Host game", Colors_Gray, programData.buttonTexture))
		{
			if (!startServer())
			{
				lastError = "Problem starting server";
			}
			else
			{
				gameStarted = true;
				if (!initGameplay(programData, nullptr))
				{
					closeServer();
					lastError = "Coultn't join, closing server";
					gameStarted = false;
				}
			}

		}
		if (programData.menuRenderer.Button("Join game", Colors_Gray,
			programData.buttonTexture))
		{
			if (initGameplay(programData, ipString))
			{
				gameStarted = true;
			}
			else
			{
				lastError = "Couldn't join server";
			}
		}
		
		programData.menuRenderer.InputText("IP: ", ipString, sizeof(ipString), 
			Colors_Gray, programData.buttonTexture, false);


		displayRenderSettingsMenuButton(programData);

		if (!lastError.empty())
		{
			programData.menuRenderer.Text(lastError, glm::vec4(1, 0, 0, 1));
		}

		programData.menuRenderer.End();
	}
	else
	{
		if (isServerRunning())
		{
			auto s = getServerSettingsCopy();

			ImGui::PushStyleColor(ImGuiCol_WindowBg, {26 / 255.f,26 / 255.f,26 / 255.f,0.5f});
			ImGui::Begin("Server window");

			ImGui::Text("Server Chunk Capacity: %d", getChunkCapacity());
			ImGui::Text("Server Ticke per seccond: %d", getServerTicksPerSeccond());
			ImGui::Text("Server Worker tick threads: %d", getThredPoolSize());

			

			for (auto &c : s.perClientSettings)
			{
				ImGui::PushID(c.first);
				ImGui::Text("%d", c.first);
				ImGui::Text("Position: %lf %lf %lf", c.second.outPlayerPos.x,
					c.second.outPlayerPos.y, c.second.outPlayerPos.z);
				ImGui::Checkbox("Allow validate moves", &c.second.validateStuff);
				ImGui::Checkbox("Spawn zombie", &c.second.spawnZombie);
				ImGui::Separator();
				ImGui::PopID();
			}


			ImGui::End();
			ImGui::PopStyleColor();

			setServerSettings(s);
		}

		if (!gameplayFrame(deltaTime, w, h, programData))
		{
			closeGameLogic();
			closeConnection();
			closeServer();		//this will do something only if the server is on
			gameStarted = false;
			platform::showMouse(true);
		}

	}



#pragma region set finishing stuff
	gl2d::enableNecessaryGLFeatures();

	programData.menuRenderer.renderFrame(programData.renderer2d, programData.font, platform::getRelMousePosition(),
		platform::isLMousePressed(), platform::isLMouseHeld(), platform::isLMouseReleased(),
		platform::isKeyReleased(platform::Button::Escape), platform::getTypedInput(), deltaTime);

	programData.renderer2d.flush();

	return true;
#pragma endregion

}

void closeGame()
{
	closeGameLogic();
	closeConnection();
	closeServer();

}
