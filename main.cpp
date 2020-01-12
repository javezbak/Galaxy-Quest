//Jonathan Avezbaki Final Project 2016 
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_mixer.h>
#include <cmath>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include <map>
#include "Entity.h"
#include "Vector3.h"
#include "ParticleEmitter.cpp"
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define MAX_BG 10

//Window 
SDL_Event event;
SDL_Window* displayWindow;
ShaderProgram* program;

//game state
enum GameState { TITLE_SCREEN, CONTROLS, LEVEL1, LEVEL2, LEVEL3, GAME_END };
int state = TITLE_SCREEN;

//matrices
Matrix projectionMatrix;
Matrix modelMatrix;
Matrix viewMatrix;

//textures
GLuint fontTexture;
Entity::ImageData spriteImgData;
Entity::ImageData selectIndicatorData;
Entity::ImageData starImgData;
Entity::ImageData healthImgData;
Entity::ImageData explosionImgData;

//Entities
Entity* planet;
Entity* selectIndicator;
Entity* player;
Entity* playerTwo;
std::vector<Entity> enemies;
std::vector<Entity> playerShotsFromEnemies;
std::vector<Entity> backgroundPieces;
std::vector<Entity> aliens;
std::vector<Entity> asteroids;
std::vector<Entity> explosions;
float timeSinceLastAlien = 0.0f;
float timeSinceLastEnemy = 0.0f;
float timeSinceLastBottomEnemy = 0.0f;
float timeSinceLastAsteroid = 0.0f;
int numEnemiesKilled = 0;
int numEnemiesKilledTwo = 0;
bool alienHit = false;
std::vector<Entity> health;
std::vector<Entity> healthTwo;
std::map <int, std::string> colors{
	{ 0, "Black" },
	{ 1, "Blue" },
	{ 2, "Green" },
	{ 3, "Red" },
};

//time keeping
float elapsed = 0.0;
float ticks = 0.0f;
float lastFrameTicks = 0.0f;
float framesPerSecond = 30.0f;
float angle = 0.0;
float elapsedUsable = 0.0f;
float starElapsed = 10.7f;
bool turn;

//title cards
bool titleCard = false;
std::string titleCardText;
float totalTitleCardTime = 0.0f;
float letterTime = 15.0f;
bool titleCardFinished = false;
bool titleCardFinishedThree = false;

bool invincibility = false;
float timeInvincible = 0.0f;

bool invincibilityTwo = false;
float timeInvincibleTwo = 0.0f;

bool screenShake = false;
float screenShakeValue = 0.01;
float screenShakeSpeed = 1.1;
float screenShakeIntensity = 0.1;

std::string playerMode = "1 player";
bool twoPlayers = false;

//player shooting
bool playerShoot;
bool playerTwoShoot;
std::vector<Entity> playerShots;
std::vector<Entity> playerTwoShots;
float shootElapsed = 10.70f; //prevents player from rapid fire
float shootElapsedTwo = 10.70f;

//animation
float animationElapsed = 0.0f;
bool nextFrame = false;
int frame = 0;

//music and sound
Mix_Chunk *laserShoot;
Mix_Chunk *ufoHit;
Mix_Chunk *select;
Mix_Chunk *newLetter;
Mix_Chunk *enemyHit;
Mix_Chunk *playerHit;
Mix_Music *mainTheme;

//function signatures
void Setup();
void Update();
void UpdateTitleScreen();
void UpdateGame();
void ProcessEvents(bool &done);
void Render();
void RenderTitleScreen();
void RenderGame();
int Cleanup();
void TimeKeep();
float TimeMove();
void TimeSinceLastShot();
void bgCheck();
void PopulateStars();
bool checkDup(float randomX, float randomY, const std::vector<Entity> ent);
void Refresh();
void addEnemies(int numOfEnemiesToAdd, bool bottom);
void generate();
void movement();
void collision();
void titleCardGeneration();
void addAsteroids(int numOfAsteroidsToAdd);
bool checkDupAlt(float randomX, float randomY, const std::vector<Entity> ents);
void RenderGameEnd();
void addExplosion(float xPos, float yPos);
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing, Vector3 translation);

int main(int argc, char *argv[])
{
	Setup();
	bool done = false;
	while (!done) {
		TimeKeep();
		elapsedUsable = TimeMove();
		ProcessEvents(done);
		Update();
		Render();
	}
	return Cleanup();
	return 0;
}

void Setup()
{
	//Window setup
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Galaxy Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1060, 680, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//view size
	glViewport(0, 0, 1060, 680);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	//Allow transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//orthographic projection
	projectionMatrix.setOrthoProjection(-7.9875f, 7.9875f, -4.0f, 4.0f, -1.0f, 1.0f);
	glUseProgram(program->programID);
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	//textures
	fontTexture = Entity::LoadImg("font.png", true).texture;
	spriteImgData = Entity::LoadImg("sheet.png", false);
	selectIndicatorData = Entity::LoadImg("select.png", true);
	starImgData = Entity::LoadImg("stars.png", true);
	healthImgData = Entity::LoadImg("health.png", true);
	explosionImgData = Entity::LoadImg("arne_sprites.png", true);

	//Entities
	selectIndicator = new Entity(Entity(selectIndicatorData, Vector3(-2.8, 1.0, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));

	planet = new Entity(Entity(Entity::LoadImg("planet.png", true), Vector3(3.2, -4.85, 0.0), Vector3(10.0, 10.0, 0.0), Vector3(), Vector3()));

	player = new Entity("sheet.png", "playerShip2_blue", spriteImgData, "sheet.xml",
		Vector3(0.0, -3.3), Vector3(), Vector3(), Vector3(), Vector3(), Vector3(0.1, 0.1, 0.0));

	//health
	health.push_back(Entity(healthImgData, Vector3(6.8, 3.52, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));
	health.push_back(Entity(healthImgData, Vector3(7.2, 3.52, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));
	health.push_back(Entity(healthImgData, Vector3(7.6, 3.52, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));

	//music and sound
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	laserShoot = Mix_LoadWAV("laser.wav");
	ufoHit = Mix_LoadWAV("land.wav");
	select = Mix_LoadWAV("button select.wav");
	newLetter = Mix_LoadWAV("new letter.wav");
	enemyHit = Mix_LoadWAV("Enemy hit.wav");
	playerHit = Mix_LoadWAV("playerHit.wav");
	mainTheme = Mix_LoadMUS("cemetery.mp3");
}

void ProcessEvents(bool &done)
{
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN && (state == LEVEL1 || state == LEVEL2 || state == LEVEL3))
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
				playerShoot = true;
			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN && playerMode == "2 players")
				playerTwoShoot = true;
		}
	}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	//quit
	if (keys[SDL_SCANCODE_ESCAPE])
		done = true;

	//cursor movement
	if (state == TITLE_SCREEN)
	{
		if (keys[SDL_SCANCODE_W])
		{
			if (selectIndicator->position.y != 1.0)
			{
				selectIndicator->position.y += 1.0;
				Mix_PlayChannel(-1, select, 0);
			}
		}
		else if (keys[SDL_SCANCODE_S])
		{
			if (selectIndicator->position.y != 0.0)
			{
				selectIndicator->position.y -= 1.0;
				Mix_PlayChannel(-1, select, 0);
			}
		}
		else if (keys[SDL_SCANCODE_D])
		{
			if (selectIndicator->position.y == 0.0 && playerMode == "1 player")
				playerMode = "2 players";
		}
		else if (keys[SDL_SCANCODE_A])
		{
			if (selectIndicator->position.y == 0.0 && playerMode == "2 players")
				playerMode = "1 player";
		}
		else if (keys[SDL_SCANCODE_SPACE])
		{
			if (selectIndicator->position.y == 1.0)
			{
				if (playerMode == "2 players")
				{
					playerTwo = new Entity("sheet.png", "playerShip2_green", spriteImgData, "sheet.xml",
						Vector3(2.0, -3.3), Vector3(), Vector3(), Vector3(), Vector3(), Vector3(0.1, 0.1, 0.0));
					twoPlayers = true;
					healthTwo.push_back(Entity(healthImgData, Vector3(6.8, 3.00, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));
					healthTwo.push_back(Entity(healthImgData, Vector3(7.2, 3.00, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));
					healthTwo.push_back(Entity(healthImgData, Vector3(7.6, 3.00, 0.0), Vector3(0.3, 0.3, 0.0), Vector3(), Vector3()));
				}
				state = LEVEL1;
				PopulateStars();
				Mix_PlayMusic(mainTheme, -1);
			}
		}
	}

	//player movement
	else if (state == LEVEL1 || state == LEVEL2 || state == LEVEL3)
	{
		if (keys[SDL_SCANCODE_W])
			player->acceleration.y = 0.012;
		if (keys[SDL_SCANCODE_S])
			player->acceleration.y = -0.012;
		if (keys[SDL_SCANCODE_A])
			player->acceleration.x = -0.012;
		if (keys[SDL_SCANCODE_D])
			player->acceleration.x = 0.012;

		if (twoPlayers)
		{
			if (keys[SDL_SCANCODE_UP])
				playerTwo->acceleration.y = 0.012;
			if (keys[SDL_SCANCODE_DOWN])
				playerTwo->acceleration.y = -0.012;
			if (keys[SDL_SCANCODE_LEFT])
				playerTwo->acceleration.x = -0.012;
			if (keys[SDL_SCANCODE_RIGHT])
				playerTwo->acceleration.x = 0.012;
		}
	}

}

void Update()
{
	glClear(GL_COLOR_BUFFER_BIT);

	switch (state)
	{
	case TITLE_SCREEN:
		UpdateTitleScreen();
		break;
	case LEVEL1:
		UpdateGame();
		break;
	case LEVEL2:
		UpdateGame();
		break;
	case LEVEL3:
		UpdateGame();
		break;
	}
}

void UpdateTitleScreen()
{


}

void UpdateGame()
{
	titleCardGeneration();

	if (timeInvincible >= 35)
	{
		invincibility = false;
		timeInvincible = 0.0f;
	}
	if (twoPlayers)
	{
		if (timeInvincibleTwo >= 35)
		{
			invincibilityTwo = false;
			timeInvincibleTwo = 0.0f;
		}
	}

	generate(); // make new entities 
	movement(); //move existing entities
	collision(); //calculate collision for entities

	bgCheck(); //update stars

	Refresh(); //remove entities when they leave the view matrix

	if (numEnemiesKilled >= 35 || (twoPlayers && numEnemiesKilledTwo >= 35))
		state = GAME_END;
}

void titleCardGeneration()
{
	//LEVEL 2
	if (totalTitleCardTime >= 80 && state == LEVEL2)
	{
		//reset variables 
		titleCardFinished = true;
		titleCard = false;
		titleCardText = "";
		totalTitleCardTime = 0.0f;
	}
	if ( (numEnemiesKilled >= 15 && titleCardFinished == false) || (twoPlayers && numEnemiesKilledTwo >= 15 && titleCardFinished == false))
	{
		state = LEVEL2;
		titleCard = true;
		letterTime += elapsedUsable;
		totalTitleCardTime += elapsedUsable;
		if (letterTime > 8.0f)
		{
			letterTime = 0.0f;
			switch (titleCardText.size())
			{
			case 0: titleCardText += "l"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 1: titleCardText += "e"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 2: titleCardText += "v"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 3: titleCardText += "e"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 4: titleCardText += "l"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 5: titleCardText += " 2"; Mix_PlayChannel(-1, newLetter, 0); break;
			}
		}
	}

	//LEVEL 3
	if (totalTitleCardTime >= 80 && state == LEVEL3)
	{
		//reset variables 
		titleCardFinishedThree = true;
		titleCard = false;
		titleCardText = "";
		totalTitleCardTime = 0.0f;
	}
	if ( (numEnemiesKilled >= 25 && titleCardFinishedThree == false) || (twoPlayers && numEnemiesKilledTwo >= 25 && titleCardFinishedThree == false) )
	{
		state = LEVEL3;
		titleCard = true;
		letterTime += elapsedUsable;
		totalTitleCardTime += elapsedUsable;
		if (letterTime > 8.0f)
		{
			letterTime = 0.0f;
			switch (titleCardText.size())
			{
			case 0: titleCardText += "l"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 1: titleCardText += "e"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 2: titleCardText += "v"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 3: titleCardText += "e"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 4: titleCardText += "l"; Mix_PlayChannel(-1, newLetter, 0); break;
			case 5: titleCardText += " 3"; Mix_PlayChannel(-1, newLetter, 0); break;
			}
		}
	}
}

void addExplosion(float xPos, float yPos, float size)
{
	explosions.push_back(Entity("arne_sprites.png", 48, 16, 8, explosionImgData, size,
		Vector3(xPos, yPos, 0.0), Vector3(), Vector3(), Vector3(), Vector3(), std::vector < int > {48, 49, 50}));
}

void generate()
{
	//shooting
	shootElapsed += elapsedUsable;
	if (playerShoot && shootElapsed >= 10.70f)
	{
		Entity laser = Entity("sheet.png", "laserBlue01.png", spriteImgData, "sheet.xml", Vector3(player->position.x, player->position.y + player->vertices[2] + 0.35, 0.0),
			Vector3(), Vector3(0.0, 0.4), Vector3(), Vector3(), Vector3());
		playerShots.push_back(laser);
		playerShoot = false;
		shootElapsed = 0.0f;
		Mix_PlayChannel(-1, laserShoot, 0);
	}

	//shooting for player Two
	if (twoPlayers)
	{
		shootElapsedTwo += elapsedUsable;
		if (playerTwoShoot && shootElapsedTwo >= 10.70f)
		{
			Entity laser = Entity("sheet.png", "laserGreen11.png", spriteImgData, "sheet.xml", Vector3(playerTwo->position.x, playerTwo->position.y + playerTwo->vertices[2] + 0.35, 0.0),
				Vector3(), Vector3(0.0, 0.4), Vector3(), Vector3(), Vector3());
			playerTwoShots.push_back(laser);
			playerTwoShoot = false;
			shootElapsedTwo = 0.0f;
			Mix_PlayChannel(-1, laserShoot, 0);
		}
	}

	//generate aliens
	timeSinceLastAlien += elapsedUsable;
	if (timeSinceLastAlien > 150) //750
	{
		float randomX = (rand() % 15) - 7.5;
		Entity alien = Entity("sheet.png", "ufoRed", spriteImgData, "sheet.xml",
			Vector3(randomX, 4.5), Vector3(), Vector3(0.0, -0.05, 0.0), Vector3(), Vector3(), Vector3(0.0, -0.1, 0.0));
		aliens.push_back(alien);
		timeSinceLastAlien = 0;
	}

	//generate enemies
	timeSinceLastEnemy += elapsedUsable;
	if (timeSinceLastEnemy > 90)
	{
		float randomChance = rand() % 10;
		if (randomChance <= 3)
			addEnemies(1, false);
		if (randomChance <= 8)
			addEnemies(2, false);
		else
			addEnemies(3, false);
		timeSinceLastEnemy = 0;
	}

	//generate asteroids
	if (state == LEVEL2 || state == LEVEL3)
	{
		timeSinceLastAsteroid += elapsedUsable;
		if (timeSinceLastAsteroid > 35)
		{
			float randomChance = rand() % 10;
			if (randomChance <= 3)
				addAsteroids(1);
			if (randomChance <= 8)
				addAsteroids(2);
			timeSinceLastAsteroid = 0;
		}
	}

	//generate ships coming from bottom of screen
	if (state == LEVEL3)
	{
		timeSinceLastBottomEnemy += elapsedUsable;
		if (timeSinceLastBottomEnemy > 65)
		{
			float randomChance = rand() % 2 + 1;
			if (randomChance <= 1)
				addEnemies(1, true);
			else
				addEnemies(2, true);
			timeSinceLastBottomEnemy = 0;
		}
	}

	//change frames for explosions
	for (int i = 0; i < explosions.size(); ++i)
	{
		explosions[i].timeOnScreen += elapsedUsable;
		if (explosions[i].timeOnScreen >= 2.0f)
			explosions[i].changeFrame(explosions[i].indecesOfAnimation[2], 16, 8);
		else if (explosions[i].timeOnScreen >= 1.0f)
			explosions[i].changeFrame(explosions[i].indecesOfAnimation[1], 16, 8);
	}

}

void movement()
{
	//move the player(s)
	player->move(elapsedUsable, true);
	if (twoPlayers)
		playerTwo->move(elapsedUsable, true);

	//move the lasers
	for (int i = 0; i < playerShots.size(); ++i)
		playerShots[i].move(elapsedUsable, false);
	if (twoPlayers)
	{
		for (int i = 0; i < playerTwoShots.size(); ++i)
			playerTwoShots[i].move(elapsedUsable, false);
	}

	//move aliens
	for (int i = 0; i < aliens.size(); ++i)
	{
		if (alienHit)
			aliens[i].move(elapsedUsable, false);
		else
			aliens[i].moveConstant(elapsedUsable);
	}

	//move enemies
	for (int i = 0; i < enemies.size(); ++i)
		enemies[i].moveConstant(elapsedUsable);

	//move asteroids
	for (int i = 0; i < asteroids.size(); ++i)
	{
		asteroids[i].moveConstant(elapsedUsable);
		asteroids[i].timeOnScreen += elapsedUsable;
		if (asteroids[i].timeOnScreen >= 150)
			asteroids[i].remove = true;
	}

	//screen Shake
	if ((timeInvincible != 0 && timeInvincible <= 20) || (twoPlayers && timeInvincibleTwo != 0 && timeInvincibleTwo <= 20))
	{
		screenShakeValue += elapsedUsable;
		viewMatrix.identity();
		viewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity,
			0.0f);
		program->setViewMatrix(viewMatrix);
	}
	else
		screenShakeValue = 0.1f;
}

void collision()
{
	//ALIEN+BULLET COLLISION
	for (int i = 0; i < playerShots.size(); ++i)
	{
		//see if any bullet collides with any enemies
		for (int y = 0; y < aliens.size(); ++y)
		{
			//collision on y & x
			if (playerShots[i].position.y + 0.5 >= aliens[y].position.y - 0.5
				&& playerShots[i].position.y - 0.5 <= aliens[y].position.y + 0.5
				&& abs(playerShots[i].position.x - aliens[y].position.x) <= 0.48)
			{
				alienHit = true;
				playerShots[i].remove = true;
				addExplosion(aliens[y].position.x, aliens[y].position.y - 0.5, 0.6);
				Mix_PlayChannel(-1, ufoHit, 0);
			}
		}
	}

	//ENEMY+BULLET COLLISION
	for (int i = 0; i < playerShots.size(); ++i)
	{
		//see if any bullet collides with any enemies
		for (int y = 0; y < enemies.size(); ++y)
		{
			//collision on y & x
			if (playerShots[i].position.y + 0.5 >= enemies[y].position.y - 0.5
				&& playerShots[i].position.y - 0.5 <= enemies[y].position.y + 0.5
				&& abs(playerShots[i].position.x - enemies[y].position.x) <= 0.48)
			{
				enemies[y].remove = true;
				playerShots[i].remove = true;
				++numEnemiesKilled;
				addExplosion(enemies[y].position.x, enemies[y].position.y, 0.85);
				Mix_PlayChannel(-1, enemyHit, 0);
			}
		}
	}

	//ASTEROID+BULLET COLLISION
	for (int i = 0; i < playerShots.size(); ++i)
	{
		//see if any bullet collides with any enemies
		for (int y = 0; y < asteroids.size(); ++y)
		{
			//collision on y & x
			if (playerShots[i].position.y + 0.5 >= asteroids[y].position.y - 0.5
				&& playerShots[i].position.y - 0.5 <= asteroids[y].position.y + 0.5
				&& abs(playerShots[i].position.x - asteroids[y].position.x) <= 0.48)
			{
				playerShots[i].remove = true;
				asteroids[y].remove = true;
				addExplosion(asteroids[y].position.x, asteroids[y].position.y, 0.85);
				Mix_PlayChannel(-1, enemyHit, 0);
			}
		}
	}

	if (!invincibility)
	{
		//PLAYER+ENEMY COLLISION
		for (int y = 0; y < enemies.size(); ++y)
		{
			//collision on y & x
			if (player->position.y + 0.5 >= enemies[y].position.y - 0.5
				&& player->position.y - 0.5 <= enemies[y].position.y + 0.5)
			{
				if (abs(player->position.x - enemies[y].position.x) <= 1.3)
				{
					if (health.size() != 0)
					{
						Mix_PlayChannel(-1, playerHit, 0);
						invincibility = true;
						health.pop_back();
						if (health.size() == 0)
							state = GAME_END;
					}
				}
			}
		}

		//PLAYER+UFO COLLISION
		for (int y = 0; y < aliens.size(); ++y)
		{
			//collision on y & x
			if (player->position.y + 0.5 >= aliens[y].position.y - 0.5
				&& player->position.y - 0.5 <= aliens[y].position.y + 0.5)
			{
				if (abs(player->position.x - aliens[y].position.x) <= 1.3)
				{
					if (health.size() != 0)
					{
						Mix_PlayChannel(-1, playerHit, 0);
						invincibility = true;
						health.pop_back();
						if (health.size() == 0)
							state == GAME_END;
					}
				}
			}
		}

		//PLAYER+ASTEROID COLLISION
		for (int y = 0; y < asteroids.size(); ++y)
		{
			//collision on y & x
			if (player->position.y + 0.5 >= asteroids[y].position.y - 0.5
				&& player->position.y - 0.5 <= asteroids[y].position.y + 0.5)
			{
				if (abs(player->position.x - asteroids[y].position.x) <= 1.3)
				{
					if (health.size() != 0)
					{
						Mix_PlayChannel(-1, playerHit, 0);
						invincibility = true;
						health.pop_back();
						if (health.size() == 0)
							state == GAME_END;
					}
				}
			}
		}
	}
	else
		timeInvincible += elapsedUsable;


	//PLAYER 2
	if (twoPlayers)
	{
		//ALIEN+BULLET COLLISION
		for (int i = 0; i < playerTwoShots.size(); ++i)
		{
			//see if any bullet collides with any enemies
			for (int y = 0; y < aliens.size(); ++y)
			{
				//collision on y & x
				if (playerTwoShots[i].position.y + 0.5 >= aliens[y].position.y - 0.5
					&& playerTwoShots[i].position.y - 0.5 <= aliens[y].position.y + 0.5
					&& abs(playerTwoShots[i].position.x - aliens[y].position.x) <= 0.48)
				{
					alienHit = true;
					playerTwoShots[i].remove = true;
					addExplosion(aliens[y].position.x, aliens[y].position.y - 0.5, 0.6);
					Mix_PlayChannel(-1, ufoHit, 0);
				}
			}
		}

		//ENEMY+BULLET COLLISION
		for (int i = 0; i < playerTwoShots.size(); ++i)
		{
			//see if any bullet collides with any enemies
			for (int y = 0; y < enemies.size(); ++y)
			{
				//collision on y & x
				if (playerTwoShots[i].position.y + 0.5 >= enemies[y].position.y - 0.5
					&& playerTwoShots[i].position.y - 0.5 <= enemies[y].position.y + 0.5
					&& abs(playerTwoShots[i].position.x - enemies[y].position.x) <= 0.48)
				{
					enemies[y].remove = true;
					playerTwoShots[i].remove = true;
					++numEnemiesKilledTwo;
					addExplosion(enemies[y].position.x, enemies[y].position.y, 0.85);
					Mix_PlayChannel(-1, enemyHit, 0);
				}
			}
		}

		//ASTEROID+BULLET COLLISION
		for (int i = 0; i < playerTwoShots.size(); ++i)
		{
			//see if any bullet collides with any enemies
			for (int y = 0; y < asteroids.size(); ++y)
			{
				//collision on y & x
				if (playerTwoShots[i].position.y + 0.5 >= asteroids[y].position.y - 0.5
					&& playerTwoShots[i].position.y - 0.5 <= asteroids[y].position.y + 0.5
					&& abs(playerTwoShots[i].position.x - asteroids[y].position.x) <= 0.48)
				{
					playerTwoShots[i].remove = true;
					asteroids[y].remove = true;
					addExplosion(asteroids[y].position.x, asteroids[y].position.y, 0.85);
					Mix_PlayChannel(-1, enemyHit, 0);
				}
			}
		}

		if (!invincibilityTwo)
		{
			//PLAYER2+ENEMY COLLISION
			for (int y = 0; y < enemies.size(); ++y)
			{
				//collision on y & x
				if (playerTwo->position.y + 0.5 >= enemies[y].position.y - 0.5
					&& playerTwo->position.y - 0.5 <= enemies[y].position.y + 0.5)
				{
					if (abs(playerTwo->position.x - enemies[y].position.x) <= 1.3)
					{
						if (healthTwo.size() != 0)
						{
							Mix_PlayChannel(-1, playerHit, 0);
							invincibilityTwo = true;
							healthTwo.pop_back();
							if (healthTwo.size() == 0)
								state = GAME_END;
						}
					}
				}
			}

			//PLAYER2+UFO COLLISION
			for (int y = 0; y < aliens.size(); ++y)
			{
				//collision on y & x
				if (playerTwo->position.y + 0.5 >= aliens[y].position.y - 0.5
					&& playerTwo->position.y - 0.5 <= aliens[y].position.y + 0.5)
				{
					if (abs(playerTwo->position.x - aliens[y].position.x) <= 1.3)
					{
						if (healthTwo.size() != 0)
						{
							Mix_PlayChannel(-1, playerHit, 0);
							invincibilityTwo = true;
							healthTwo.pop_back();
							if (healthTwo.size() == 0)
								state == GAME_END;
						}
					}
				}
			}

			//PLAYER2+ASTEROID COLLISION
			for (int y = 0; y < asteroids.size(); ++y)
			{
				//collision on y & x
				if (playerTwo->position.y + 0.5 >= asteroids[y].position.y - 0.5
					&& playerTwo->position.y - 0.5 <= asteroids[y].position.y + 0.5)
				{
					if (abs(playerTwo->position.x - asteroids[y].position.x) <= 1.3)
					{
						if (healthTwo.size() != 0)
						{
							Mix_PlayChannel(-1, playerHit, 0);
							invincibilityTwo = true;
							healthTwo.pop_back();
							if (healthTwo.size() == 0)
								state == GAME_END;
						}
					}
				}
			}
		}
		else
			timeInvincibleTwo += elapsedUsable;
	}

	if (health.size() == 0 || (twoPlayers && healthTwo.size() == 0))
		state = GAME_END;
}

void PopulateStars()
{
	while (backgroundPieces.size() < 10)
	{
		int random = rand() % 100;
		if (random < 60)
		{
			float randomX = (rand() % 14) - 7;
			float randomY = (rand() % 8) - 4;
			Entity smallStar = Entity("star.png", 0, 2, 3, starImgData, 1.0,
				Vector3(randomX, randomY, 0.0), Vector3(), Vector3(0.0, -0.01, 0.0), Vector3(), Vector3(), std::vector < int > {0, 1});
			if (checkDup(randomX, randomY, backgroundPieces))
				backgroundPieces.push_back(smallStar);
		}
		else
		{
			float randomX = (rand() % 14) - 7;
			float randomY = (rand() % 8) - 4;
			Entity largeStar = Entity("star.png", 2, 2, 3, starImgData, 1.0,
				Vector3(randomX, randomY, 0.0), Vector3(), Vector3(0.0, -0.01, 0.0), Vector3(), Vector3(), std::vector < int > {2, 3});
			if (checkDup(randomX, randomY, backgroundPieces))
				backgroundPieces.push_back(largeStar);
		}
	}
}

void addEnemies(int numOfEnemiesToAdd, bool bottom)
{
	int enemiesAdded = 0;
	while (enemiesAdded < numOfEnemiesToAdd)
	{
		//generate one enemy
		int randColor = rand() % 4;
		int randShip = rand() % 5 + 1;

		float xPos = (rand() % 14) - 7;
		float yPos;
		float yVel;
		float rotation;

		if (!bottom)
		{
			yPos = 4.5f;
			yVel = -0.05f;
			rotation = 0.0f;
		}
		else
		{
			yPos = -4.5f;
			yVel = 0.05f;
			rotation = (180 * M_PI) / 180;
		}

		std::string ship = "enemy" + colors[randColor] + std::to_string(randShip) + ".png";

		if (checkDup(xPos, yPos, enemies) && checkDup(xPos, yPos, aliens))
		{
			enemies.push_back(Entity("sheet.png", ship, spriteImgData, "sheet.xml",
				Vector3(xPos, yPos), Vector3(), Vector3(0.0, yVel, 0.0), Vector3(rotation, 0.0f, 0.0f), Vector3(), Vector3()));

			++enemiesAdded;
		}
	}
}

void addAsteroids(int numOfAsteroidsToAdd)
{
	int asteroidsAdded = 0;
	while (asteroidsAdded < numOfAsteroidsToAdd)
	{
		//generate one enemy
		int randColor = rand() % 2 + 1;
		std::string color = ((randColor == 1) ? "Brown_big" : "Grey_big");
		int randNum = rand() % 4 + 1;

		float posNeg = rand() % 2 + 1;
		float xPos = posNeg == 1 ? -8.5 : 8.5;
		float yPos = rand() % 8 + 1;
		yPos -= 4;

		float xVel = xPos < 0 ? 0.1 : -0.1;
		float yVel = yPos <= 0 ? 0.1 : -0.1;

		std::string asteroid = "meteor" + color + std::to_string(randNum) + ".png";

		if (checkDupAlt(xPos, yPos, asteroids))
		{
			asteroids.push_back(Entity("sheet.png", asteroid, spriteImgData, "sheet.xml",
				Vector3(xPos, yPos), Vector3(), Vector3(xVel, yVel, 0.0), Vector3(), Vector3(), Vector3()));

			++asteroidsAdded;
		}
	}
}

bool checkDup(float randomX, float randomY, const std::vector<Entity> ents)
{
	for (int i = 0; i < ents.size(); ++i)
	{
		if (ents[i].position.x == randomX && ents[i].position.y == randomY)
			return false;
	}
	return true;
}

bool checkDupAlt(float randomX, float randomY, const std::vector<Entity> ents)
{
	for (int i = 0; i < ents.size(); ++i)
	{
		if (abs(ents[i].position.y - randomY) < 2.0 && abs(ents[i].position.x - randomX) < 0.2)
			return false;
	}
	return true;
}

void Refresh()
{
	//remove aliens that aren't in the view matrix
	std::vector<Entity>::iterator iterAlien;

	for (iterAlien = aliens.begin(); iterAlien != aliens.end();) {
		if (iterAlien->position.y < -4.5)
		{
			alienHit = false;
			iterAlien = aliens.erase(iterAlien);
		}
		else
			++iterAlien;
	}

	//remove stars that aren't in the view matrix
	std::vector<Entity>::iterator iterStar;
	for (iterStar = backgroundPieces.begin(); iterStar != backgroundPieces.end();) {
		if (iterStar->position.y < -4.5)
			iterStar = backgroundPieces.erase(iterStar);
		else
			++iterStar;
	}

	//remove enemies that aren't in the view matrix
	std::vector<Entity>::iterator iterEnemy;
	for (iterEnemy = enemies.begin(); iterEnemy != enemies.end();) {
		if (iterEnemy->position.y < -4.5 || iterEnemy->remove)
			iterEnemy = enemies.erase(iterEnemy);
		else
			++iterEnemy;
	}

	//remove lasers not in the view matrix
	std::vector<Entity>::iterator iterPlayerShot;
	for (iterPlayerShot = playerShots.begin(); iterPlayerShot != playerShots.end();) {
		if (iterPlayerShot->position.y > 4.5 || iterPlayerShot->remove)
			iterPlayerShot = playerShots.erase(iterPlayerShot);
		else
			++iterPlayerShot;
	}

	if (twoPlayers)
	{
		//remove lasers not in the view matrix
		std::vector<Entity>::iterator iterPlayerShotTwo;
		for (iterPlayerShotTwo = playerTwoShots.begin(); iterPlayerShotTwo != playerTwoShots.end();) {
			if (iterPlayerShotTwo->position.y > 4.5 || iterPlayerShotTwo->remove)
				iterPlayerShotTwo = playerTwoShots.erase(iterPlayerShotTwo);
			else
				++iterPlayerShotTwo;
		}
	}

	//remove asteroids not in the view matrix
	std::vector<Entity>::iterator iterAsteroid;
	for (iterAsteroid = asteroids.begin(); iterAsteroid != asteroids.end();) {
		if (iterAsteroid->remove)
			iterAsteroid = asteroids.erase(iterAsteroid);
		else
			++iterAsteroid;
	}

	//remove explosions after a certain amount of time 
	std::vector<Entity>::iterator iterExplosion;
	for (iterExplosion = explosions.begin(); iterExplosion != explosions.end();) {
		if (iterExplosion->timeOnScreen >= 3.0f)
			iterExplosion = explosions.erase(iterExplosion);
		else
			++iterExplosion;
	}

}

void bgCheck()
{

	//add new stars as needed
	float lastPlaced;
	while (backgroundPieces.size() < 15)
	{
		int random = rand() % 100;
		if (random < 50)
		{
			float randomX = (rand() % 14) - 7;
			Entity smallStar = Entity("star.png", 0, 2, 3, starImgData, 1.0,
				Vector3(randomX, 4.5, 0.0), Vector3(), Vector3(0.0, -0.01, 0.0), Vector3(), Vector3(), std::vector < int > {0, 1});
			if (checkDup(randomX, 4.5, backgroundPieces))
				backgroundPieces.push_back(smallStar);
		}
		else if (random < 85)
		{
			float randomX = (rand() % 14) - 7;
			Entity largeStar = Entity("star.png", 2, 2, 3, starImgData, 1.0,
				Vector3(randomX, 4.5, 0.0), Vector3(), Vector3(0.0, -0.01, 0.0), Vector3(), Vector3(), std::vector < int > {2, 3});
			if (checkDup(randomX, 4.5, backgroundPieces))
				backgroundPieces.push_back(largeStar);
		}
	}

	//move stars
	for (int i = 0; i < backgroundPieces.size(); ++i)
	{
		backgroundPieces[i].moveConstant(elapsedUsable);
	}

	//animate stars
	starElapsed += elapsedUsable;
	if (starElapsed >= 30.7)
	{
		for (int i = 0; i < backgroundPieces.size(); ++i)
		{
			backgroundPieces[i].changeFrame(backgroundPieces[i].indecesOfAnimation[frame], 2, 3);
		}
		if (frame == 0)
			frame = 1;
		else if (frame == 1)
			frame = 0;
		starElapsed = 0.0f;
	}
}

void Render()
{
	switch (state)
	{
	case TITLE_SCREEN:
		RenderTitleScreen();
		break;
	case LEVEL1:
		RenderGame();
		break;
	case LEVEL2:
		RenderGame();
		break;
	case LEVEL3:
		RenderGame();
		break;
	case GAME_END:
		RenderGameEnd();
		break;
	}

	SDL_GL_SwapWindow(displayWindow);
}

void RenderTitleScreen()
{
	//planet
	planet->Draw(program, modelMatrix);

	//text
	DrawText(program, fontTexture, "galaxy run", 1.0f, 0.0f, Vector3(-4.50, 3.5, 0.0));
	DrawText(program, fontTexture, "start", 0.5f, 0.0f, Vector3(-2.40, 1.0, 0.0));
	DrawText(program, fontTexture, playerMode, 0.5f, 0.0f, Vector3(-2.40, 0.0, 0.0));

	//select key
	selectIndicator->Draw(program, modelMatrix);
}

void RenderGame()
{
	//stars
	for (int i = 0; i < backgroundPieces.size(); ++i)
		backgroundPieces[i].Draw(program, modelMatrix);

	//player(s)
	player->Draw(program, modelMatrix);
	if (twoPlayers)
		playerTwo->Draw(program, modelMatrix);

	//lasers
	for (int i = 0; i < playerShots.size(); ++i)
		playerShots[i].Draw(program, modelMatrix);
	for (int i = 0; i < playerTwoShots.size(); ++i)
		playerTwoShots[i].Draw(program, modelMatrix);

	//aliens
	for (int i = 0; i < aliens.size(); ++i)
		aliens[i].Draw(program, modelMatrix);

	//enemies
	for (int i = 0; i < enemies.size(); ++i)
		enemies[i].Draw(program, modelMatrix);

	//asteroids
	for (int i = 0; i < asteroids.size(); ++i)
		asteroids[i].Draw(program, modelMatrix);

	//explosions
	for (int i = 0; i < explosions.size(); ++i)
		explosions[i].Draw(program, modelMatrix);

	DrawText(program, fontTexture, std::to_string(numEnemiesKilled) + " kills", 0.6f, 0.0f, Vector3(-7.42, 3.52, 0.0));
	DrawText(program, fontTexture, "life", 0.6f, 0.0f, Vector3(4.5, 3.52, 0.0));
	for (int i = 0; i < health.size(); ++i)
	{
		health[i].Draw(program, modelMatrix);
	}

	if (twoPlayers)
	{
		DrawText(program, fontTexture, std::to_string(numEnemiesKilledTwo) + " kills", 0.6f, 0.0f, Vector3(-7.42, 3.00, 0.0));
		DrawText(program, fontTexture, "life", 0.6f, 0.0f, Vector3(4.5, 3.00, 0.0));
		for (int i = 0; i < healthTwo.size(); ++i)
		{
			healthTwo[i].Draw(program, modelMatrix);
		}
	}

	if ((state == LEVEL2 || state == LEVEL3) && titleCard)
	{
		DrawText(program, fontTexture, titleCardText, 0.8f, 0.0f, Vector3(-2.40, 1.0, 0.0));
	}

}

void RenderGameEnd()
{
	if (twoPlayers)
	{
		if (health.size() == 0 && healthTwo.size() != 0)
			DrawText(program, fontTexture, "player 2 wins", 1.0f, 0.0f, Vector3(-6.0, 0.0, 0.0));
		else if (healthTwo.size() == 0 && health.size() != 0)
			DrawText(program, fontTexture, "player 1 wins", 1.0f, 0.0f, Vector3(-6.0, 0.0, 0.0));
		else if (numEnemiesKilled > numEnemiesKilledTwo)
			DrawText(program, fontTexture, "player 1 wins", 1.0f, 0.0f, Vector3(-6.0, 0.0, 0.0));
		else
			DrawText(program, fontTexture, "player 2 wins", 1.0f, 0.0f, Vector3(-6.0, 0.0, 0.0));
	}
	else
	{
		if (health.size() != 0)
			DrawText(program, fontTexture, "you win", 1.0f, 0.0f, Vector3(-3.50, 0.0, 0.0));
		else
			DrawText(program, fontTexture, "you lose", 1.0f, 0.0f, Vector3(-3.50, 0.0, 0.0));
	}
	
}

int Cleanup()
{
	Mix_FreeChunk(laserShoot);
	Mix_FreeChunk(ufoHit);
	Mix_FreeChunk(select);
	Mix_FreeChunk(newLetter);
	Mix_FreeChunk(enemyHit);
	Mix_FreeChunk(playerHit);
	Mix_FreeMusic(mainTheme);
	SDL_Quit();
	return 0;
}

//Helper Functions
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing, Vector3 translation) {
	float texture_size_x = 1.0 / 9.0f;
	float texture_size_y = 1.0 / 4.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		int textNum = ((int)text[i] - 97);
		switch (text[i])
		{
			//numbers and spaces don't align correctly with the font image
		case ' ': textNum = 26; break;
		case '0': textNum = 14; break;
		case '1': textNum = 27; break;
		case '2': textNum = 28; break;
		case '3': textNum = 29; break;
		case '4': textNum = 30; break;
		case '5': textNum = 31; break;
		case '6': textNum = 32; break;
		case '7': textNum = 33; break;
		case '8': textNum = 34; break;
		case '9': textNum = 35; break;
		}
		float texture_x = (float)(textNum % 9) / 9.0f;
		float texture_y = (float)(textNum / 9) / 4.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size_y,
			texture_x + texture_size_x, texture_y,
			texture_x + texture_size_x, texture_y + texture_size_y,
			texture_x + texture_size_x, texture_y,
			texture_x, texture_y + texture_size_y,
		});
	}
	modelMatrix.identity();
	modelMatrix.Translate(translation.x, translation.y, translation.z);
	program->setModelMatrix(modelMatrix);
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void TimeKeep()
{
	//general time keep
	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	if (elapsed == 0.0f)
		ticks = 0.0;
}

float TimeMove()
{
	//Time
	float fixedElapsed = elapsed;
	if (fixedElapsed > (FIXED_TIMESTEP * MAX_TIMESTEPS))
		fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;

	while (fixedElapsed > FIXED_TIMESTEP)
	{
		fixedElapsed -= FIXED_TIMESTEP;
		return FIXED_TIMESTEP;
	}
	while (fixedElapsed < FIXED_TIMESTEP)
	{
		fixedElapsed += FIXED_TIMESTEP;
		return FIXED_TIMESTEP;
	}
	return fixedElapsed;
}

