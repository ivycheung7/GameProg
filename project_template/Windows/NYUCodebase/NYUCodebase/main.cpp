#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>
#include <string>
#include <math.h>
#include <SDL_mixer.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

using namespace std;
SDL_Window* displayWindow;
#define FIXED_TIMESTEP 0.0166666f 
#define MAX_TIMESTEPS 6
#define LEVEL_HEIGHT 13
#define LEVEL_WIDTH 10
#define BLOCK_LENGTH 0.5f
unsigned char level1Data[LEVEL_HEIGHT][LEVEL_WIDTH] = {	
	{ 0, 0, 0, 0, 0, 6, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 6, 6, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 6, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 6, 0, 0, 0 },
	{ 6, 6, 6, 6, 0, 0, 0, 6, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 6, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 },
	{ 6, 6, 6, 6, 0, 0, 0, 0, 0, 6 }
};


GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) + (-0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f * size, ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size), 0.5f * size, ((size + spacing) * i) + (-0.5f * size), -0.5f * size, });         texCoordData.insert(texCoordData.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x + texture_size, texture_y, texture_x + texture_size, texture_y + texture_size, texture_x + texture_size, texture_y, texture_x, texture_y + texture_size, });
	}
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

class Entity{
public:
	float x, y, height, width, speed, directionX, directionY;
	bool collideTop, collideBottom, collideLeft, collideRight;
	float velocity_x;   
	float velocity_y;   
	float acceleration_x;   
	float acceleration_y;
	//directionX is left/right
	//directionY is down/up
	Matrix matrix;
	bool isAlive; //Living player vs block
	int textureID;
	int spriteCountX, spriteCountY; 
	bool isStatic; //static: no gravity, no movement, no collision checking
	Entity(){}
	void player(){
		x = 0.0f;
		y = 0.0f;
		height = .75f;
		width = 1.0f;
		speed = 0.0f;
		directionX = 0.0f;
		directionY = 0.0f;
//		active = true;
		isAlive = true;
		textureID = LoadTexture("toilets.png");
		spriteCountX = 7;
		spriteCountY = 1;
		isStatic = false;
		collideTop = false;
		collideBottom = false;
		collideLeft = false;
		collideRight = false;
		velocity_x = 0;
		velocity_y = 0;
		acceleration_x = 0;
		acceleration_y = 0;

	}

	void blocks(float xPos, float yPos){
		x = xPos;
		y = yPos;
		height = 0.5f;
		width = 0.5f;
		speed = 1.0f;
		directionX = 0.0f;
		directionY = 0.0f;
	//	active = true;
		isStatic = true;
		isAlive = false;
		textureID = LoadTexture("goldblock.png");
		collideTop = false;
		collideBottom = false;
		collideLeft = false;
		collideRight = false;
	}
	void resetFlags(){
		collideTop = false;
		collideBottom = false;
		collideLeft = false;
		collideRight = false;
	}
	void updateImage(float texVert[],ShaderProgram * program){
		GLfloat vertices[] =
		{
			x - width / 2, y - height / 2,
			x + width / 2, y - height / 2,
			x + width / 2, y + height / 2,
			x - width / 2, y - height / 2,
			x + width / 2, y + height / 2,
			x - width / 2, y + height / 2
		};
		program->setModelMatrix(matrix);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glBindTexture(GL_TEXTURE_2D, textureID);

		float texCoords[] = { 
			0.0, 1.0, 
			1.0, 1.0, 
			1.0, 0.0, 
			0.0, 1.0, 
			1.0, 0.0, 
			0.0, 0.0 
		};

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texVert);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	
	}
	
	//	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	//	glEnableVertexAttribArray(program->texCoordAttribute);

	//	glBindTexture(GL_TEXTURE_2D, textureID);
	//	glDrawArrays(GL_TRIANGLES, 0, 6);
	//		//	return texCoords[];
	//		//	float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
	//		// our regular sprite drawing 

	//	}
	void DrawSpriteSheetSprite(ShaderProgram *program, int index) {
		float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
		float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0 / (float)spriteCountX;
		float spriteHeight = 1.0 / (float)spriteCountY;

		if (!directionX){
			//If negativ and moving to the left side
			GLfloat texCoords[] =
			{
				u, v + spriteHeight,
				u + spriteWidth, v + spriteHeight,
				u + spriteWidth, v,
				u, v + spriteHeight,
				u + spriteWidth, v,
				u, v //0,0
			};

		}
		GLfloat texCoords[] =
		{
			u, v + spriteHeight,
			u + spriteWidth, v + spriteHeight,
			u + spriteWidth, v,
			u, v + spriteHeight,
			u + spriteWidth, v,
			u, v //0,0
		};
		float vertices[] = { 
			x - width / 2, y - height / 2,
			x + width / 2, y - height / 2,
			x + width / 2, y + height / 2,
			x - width / 2, y - height / 2,
			x + width / 2, y + height / 2,
			x - width / 2, y + height / 2
		};

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	void updateImage(ShaderProgram * program, int index, vector<Entity>entities){
		for (Entity e : entities){
			if (collidesWith(&e)){
				//if there is a collision, check what kind it is
		//		if (collideTop){
		//			y -= fabs((y + height / 2) - (e.y - e.height / 2)) + .001f;
		//		}
				if (collideBottom){
					y += (fabs((y - height / 2) - (e.y + e.height / 2))) + .001f;
				}
		//		else if (collideLeft){
		//			x += fabs((x - width / 2) - (e.x + e.width / 2)) + .001f;
		//		}
		//		else if (collideRight){
		//			x -= (fabs((x + width / 2) - (e.x - e.width / 2))) - .001f;
		//		}
			}
		}
		DrawSpriteSheetSprite(program, index);
	}
	bool collidesWith(Entity *entity){
	//true if collision happened
		//sets variables true or false
		//Left side collision
		/*if ((x + width/2) > (entity->x - entity->width/2)){
			if (((y + height / 2) < (entity->y + height / 2) &&
				((y + height / 2) > (entity->y - height / 2))) ||
				((y - height / 2) > (entity->y - height / 2) &&
				(y - height / 2) < (entity->y + height / 2))){
				collideLeft = true;
				return true;
			}
		}*/
		//if ((x + width / 2) > (entity->x - entity->width / 2)){
		//	//Right side collision
		//	if (((y + height / 2) < (entity->y + height / 2) &&
		//		((y + height / 2) > (entity->y - height / 2))) ||
		//		((y - height / 2) > (entity->y - height / 2) &&
		//		(y - height / 2) < (entity->y + height / 2))){
		//		collideRight = true;
		//		return true;
		//	}
		//}
		if ((y - height/ 2) < (entity->y + entity->height/ 2)){
			//Bottom collision
			if (((x + width/ 2) < (entity->x + width/ 2) &&
				((x + width/ 2) > (entity->x - width/ 2))) ||
				((x - width / 2) > (entity->x - width / 2) &&
				(x - width / 2) < (entity->x + width / 2))){
					collideBottom = true;
					return true;
			}
		}
		//if ((y + height / 2) > (entity->y - entity->height / 2)){
		//	//Top collision
		//	if (((x + width / 2) < (entity->x + width / 2) &&
		//		((x + width / 2) > (entity->x - width / 2))) ||
		//		((x - width / 2) > (entity->x - width / 2) &&
		//		(x - width / 2) < (entity->x + width / 2))){
		//		collideRight = true;
		//		return true;
		//	}
		//}
		else{
			collideTop = false;
			collideBottom = false;
			collideLeft = false;
			collideRight = false;
			return false;

		}
	}
};

float lerp(float v0, float v1, float t)
{
	return (1.0f - t)*v0 + t*v1;
}

bool collision(Entity* l, Entity* r){
	if (l->y - l->height / 2 < r->y + r->height / 2 && l->y + l->height / 2 > r->y - r->height / 2 &&
		l->x - l->width / 2 < r->x + r->width / 2 && l->x + l->width / 2 > r->x - r->width / 2)
		return true;
	return false;
}
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("HW5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.3f, 0.5f, 1.0f);

	SDL_Event event;
	bool done = false;


	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glUseProgram(program.programID);

	Matrix modelMatrix;
	Matrix modelMatrixc;
	Matrix viewMatrix;
	Matrix startMenu;

	Matrix projectionMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	Entity toilet;
	toilet.player();
	vector<Entity> entities;
	Entity block1;
	block1.blocks(0.0f, -1.5f);
	Entity block2;
	block2.blocks(0.5f, -1.5f);
	Entity block3;
	block3.blocks(1.0f, -1.5f);
	entities.push_back(block1);
	entities.push_back(block2);
	entities.push_back(block3);
	float friction_x = .5f;
	float friction_y = .1f;
	//float verticesP1[] = { -3.5f, -1.0f, -2.0f, -1.0f, -2.0f, 0.25f, -3.5f, -1.0f, -2.0f, 0.25f, -3.5f, 0.25f };
	float verticesP1[] = { -1.5f, -1.0f, 0.0f, -1.0f, 0.0f, 0.25f, -1.5f, -1.0f, 0.0f, 0.25f, -1.5f, 0.25f };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	//int winner = 0;
	const int runAnimation[] = { 0, 1, 2, 3, 4, 5, 6 };
	//{3,4,5, 15,16,17, 27,28,29, 39,40,41}
	//{6,7,8, 18,19,20, 30,31,32, 42,43,45}
	const int numFrames = 7;
	float animationElapsed = 0.0f;
	float framesPerSecond = 30.0f;
	int currentIndex = 0;
	float lastFrameTicks = 0.0f;
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;         
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) { 
			if (level1Data[y][x] == 6) {          
				//if 6, put block
				Entity block1;
				block1.blocks(y * BLOCK_LENGTH, x * BLOCK_LENGTH);
				entities.push_back(block1);
			} 
		} 
	}
	
	while (!done) {
		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		//glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesP1);
		glEnableVertexAttribArray(program.positionAttribute);
		glEnableVertexAttribArray(program.texCoordAttribute);
		//glBindTexture(GL_TEXTURE_2D, kyoukoIDLE);
		//startMenu.setScale(4, 4, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		float ticks = (float)SDL_GetTicks() / 1000;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
		}

		toilet.velocity_x += toilet.acceleration_x *FIXED_TIMESTEP;
		toilet.velocity_y -= .01f * elapsed;
		toilet.x += toilet.velocity_x * FIXED_TIMESTEP;
		toilet.y += toilet.velocity_y *FIXED_TIMESTEP;
		
		toilet.matrix.Translate(toilet.velocity_x* FIXED_TIMESTEP, toilet.velocity_x* FIXED_TIMESTEP, 0.0f);

		for (Entity e : entities){
			e.updateImage(texCoords, &program);
		};


		//DrawSpriteSheetSprite(&program, runAnimation[currentIndex], 7, 1, verticesP1, toilet);
		viewMatrix.setPosition(-toilet.x, -toilet.y, 0);

		if (keys[SDL_SCANCODE_A]) {
			animationElapsed += fixedElapsed;
			if (animationElapsed > 1.0 / framesPerSecond) {
				currentIndex++;
				animationElapsed = 0.0;
				if (currentIndex > numFrames - 1) {
					currentIndex = 0;
				}
			}
			toilet.x -= framesPerSecond * fixedElapsed / 5;
			toilet.updateImage(&program, runAnimation[currentIndex], entities);
		}
		else if (keys[SDL_SCANCODE_D]) {
			animationElapsed += fixedElapsed;
			if (animationElapsed > 1.0 / framesPerSecond) {
				currentIndex++;
				animationElapsed = 0.0;
				if (currentIndex > numFrames - 1) {
					currentIndex = 0;
				}
			}
			toilet.x += framesPerSecond * fixedElapsed / 5;
			toilet.updateImage(&program, runAnimation[currentIndex], entities);
		}
		else{
			currentIndex = 4;
			toilet.updateImage(&program, runAnimation[currentIndex], entities);
		}
		//else if (keys[SDL_SCANCODE_D]) {
		//	if (toilet.x + (framesPerSecond * fixedElapsed) < 1.50f){
		//		toilet.x += framesPerSecond * fixedElapsed;
		//		//	magikarp.matrix.Translate(0, -framesPerSecond * elapsed, 0);
		//	}
		//}

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			/*
			if (keys[SDL_SCANCODE_W] && toilet.collideBottom){
				toilet.y += framesPerSecond * FIXED_TIMESTEP;
				toilet.updateImage(&program, runAnimation[currentIndex], entities);
			}*/
			if (keys[SDL_SCANCODE_W]){
				toilet.y += framesPerSecond * FIXED_TIMESTEP;
				toilet.updateImage(&program, runAnimation[currentIndex], entities);
			}
		}
		//	glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(displayWindow);
	}
	SDL_Quit();
	return 0;
}