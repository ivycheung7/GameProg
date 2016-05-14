#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <math.h>
#include <vector>
#include "PerlinNoise.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
using namespace std;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}

float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

void DrawText(ShaderProgram *program, Matrix& matrix, int fontTexture, string text, float size, float spacing, float x, float y) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;

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
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	program->setModelMatrix(matrix);
	matrix.setPosition(x, y, 0.0f);

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

class Entity{
public:
	float x, y, height, width, speed, directionX, directionY;
	Matrix matrix;
	bool isAlive;
	bool active; // on screen
	int textureID;
	bool collidedBottom, collidedTop, collidedLeft, collidedRight;

	Entity(){}

	void player(){
		x = 0.0f;
		y = 0.0f;
		height = 2.0f;
		width = 1.0f;
		speed = 1.0f;
		directionX = 0.0f;
		directionY = 0.0f;
		active = true;
		isAlive = true;
		textureID = LoadTexture("dieho.png");
		collidedBottom = false;
		collidedTop = false;
		collidedLeft = false;
		collidedRight = false;
	}
	void bullet(){
		x = -5.0f;
		y = -0.29;
		height = 0.2f;
		width = 0.4f;
		speed = 1.0f;
		directionX = 1.0f;
		directionY = 0.0f;
		active = false;
		isAlive = true;
		textureID = LoadTexture("sword.png");
	}
	void enemy(){
		x = 2.00f;
		y = 1.75f;
		height = 0.5f;
		width = 0.5f;
		speed = 0.5f;
		directionX = 0.0f;
		directionY = -1.0f;
		isAlive = true;
		active = true;
		textureID = LoadTexture("onigiri.png");
	}
	void tile(){
		x = 1.0f;
		y = 1.0f;
		height = 1.0f;
		width = 1.0f;
		speed = 0.5f;
		directionX = 0.0f;
		directionY = -1.0f;
		isAlive = true;
		active = true;
		textureID = LoadTexture("dieho.png");
	}
	void updateImage(ShaderProgram * program, float* texVert){
		if (active){
			if (!isAlive){
				//Dead so go off screen
				active = false;
			}
			GLfloat vertices[] =
			{
				x - width / 2, y - height / 2,
				x + width / 2, y - height / 2,
				x + width / 2, y + height / 2,
				x - width / 2, y - height / 2,
				x + width / 2, y + height / 2,
				x - width / 2, y + height / 2 //0,0
			};

			program->setModelMatrix(matrix);
			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program->positionAttribute);
			glBindTexture(GL_TEXTURE_2D, textureID);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texVert);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	bool collidesWith(Entity* block) {
		/*
		is player’s bottom higher than block’s top ?
		is player’s top lower than block’s bottom ?
		is player’s left larger than block’s right ?
		is player’s right smaller than block’s left ?
		If ANY of the above are true, then the two rectangles are NOT intersecting!
		The rectangles are intersecting if NONE of the above are true.
		*/

		float top = y + height / 2;
		float bot = y - height / 2;
		float left = x- width / 2;
		float right = x+ width / 2;

		float blockTop = block->y+ block->height / 2;
		float blockBot = block->y- block->height / 2;
		float blockLeft = block->x- block->width / 2;
		float blockRight = block->x + block->width / 2;

		/*if (yPos - height / 2 < block->yPos + block->height / 2 &&
		yPos + height / 2 > block->yPos - block->height / 2 &&
		xPos - width / 2 < block->xPos + block->width / 2 &&
		xPos + width / 2 > block->xPos - block->width / 2)*/

		if (bot < blockTop && top > blockBot && left < blockRight && right > blockLeft)
			return true; //there's a collision
		return false;
	}
};

struct Pair{
	float x;
	float y;

};


struct Color{
	Color(int i = 255, int j = 255, int k = 255, int l = 0){
		r = i;
		g = j;
		b = k;
		a = l;
	}

	int r;
	int g;
	int b;
	int a;
};
struct Particle{
	Pair position;
	Pair velocity;
	float lifetime;

	float width = .5f;
	float height =.5f;
	Matrix matrix; 

	float sizeDeviation;
	float perlinY;
	bool isDead;
};

class ParticleEmitter{
public:
	int particleTexture;
	Pair position;
	Pair gravity; // particle's acceleration
	Pair velocity;
	Pair velocityDeviation;
	float maxLifetime;
	
	float startSize;
	float endSize;
	float sizeDeviation;
	float perlinSize;

	bool stopFlow;
	vector<Particle> particles;

	ParticleEmitter(unsigned int particleCount){
		stopFlow = false;
		maxLifetime = 3;
		position.x = 0.0f;
		position.y = 0.0f;
		velocity.x = 3.0f;
		velocity.y = 5.0f;
		velocityDeviation.x = 2.0f;
		velocityDeviation.y = 2.0f;
		startSize = 0.1f;
		endSize = 1.0f;
		sizeDeviation = .5;
		perlinSize = 1.0f;
		particleTexture = LoadTexture("bun.png");

		for (int i = 0; i < particleCount; i++){
			//Random since we dont want particles to reset together
			Particle p;
			randomize(p);
			particles.push_back(p);
		}
	}
	//ParticleEmitter();
	//~ParticleEmitter();

	void randomize(Particle& p){
		p.perlinY = rand() / (RAND_MAX);
		p.lifetime = rand() / (RAND_MAX / maxLifetime);
		p.position.x = position.x;
		p.position.y = position.y;
		p.sizeDeviation = rand() / (RAND_MAX / sizeDeviation);
		p.isDead = false;
		//Both directions
		//		p.velocity.x = velocity.x + ((rand() / (RAND_MAX / velocityDeviation.x*.5)) - velocityDeviation.x);
		//		p.velocity.y = velocity.y + ((rand() / (RAND_MAX / velocityDeviation.y*.5)) - velocityDeviation.y);
		//One direction! Im not a fan, i swear. Im a human.

		p.velocity.x = velocity.x + rand() / (RAND_MAX / velocityDeviation.x);
		p.velocity.y = velocity.y + rand() / (RAND_MAX / velocityDeviation.y) + .5;
	}
	void resetAll(){
		//Resets all particles to be alive and randomizes attributes
		for (Particle p : particles){
			ressurect(p);
			randomize(p);
		}
	}
	void kill(Particle& p){
		//Kills it, and makes the width and height super tiny. If 0, then the render crashes (?)
		p.isDead = true;
		p.width = 0.001;
		p.height = 0.001;
	}
	void ressurect(Particle& p){
		p.isDead = false;
		p.width = .5f;
		p.height = .5f;

	}
	void setStopFlow(bool b){
		stopFlow = b;
		if (!stopFlow){
			resetDead();
		}
	}
	void Update(float elapsed){
		//adjust velocity and position based on elapsed time 
		float perlinValue = 0.0f;
		for (int i = 0; i < particles.size(); i++){
			/*
			perlinValue += elapsed;
			float coord[2] = { perlinValue, particles[i].perlinY };
			particles[i].position.x += noise2(coord) * perlinSize;
			coord[0] = perlinValue * 0.5f;
			particles[i].position.y += noise2(coord) * perlinSize;
			*/
			particles[i].lifetime += elapsed;
			if (particles[i].lifetime > maxLifetime){
				randomize(particles[i]);
			
				//not sure if particles[i].position = position; does the same job
			}
			else{
				particles[i].position.x += elapsed * particles[i].velocity.x;
				particles[i].position.y += elapsed * particles[i].velocity.y;
				particles[i].velocity.y += -9.8 * elapsed;
			}
		}
	}
	void randomizeAttributes(Particle& p){
		p.perlinY = rand() / (RAND_MAX);
		p.lifetime = rand() / (RAND_MAX / maxLifetime);
		p.position.x = position.x;
		p.position.y = position.y;
		p.sizeDeviation = rand() / (RAND_MAX / sizeDeviation);

		//Both directions
		//		p.velocity.x = velocity.x + ((rand() / (RAND_MAX / velocityDeviation.x*.5)) - velocityDeviation.x);
		//		p.velocity.y = velocity.y + ((rand() / (RAND_MAX / velocityDeviation.y*.5)) - velocityDeviation.y);
		//One direction! Im not a fan, i swear. Im a human.

		p.velocity.x = velocity.x + rand() / (RAND_MAX / velocityDeviation.x);
		p.velocity.y = velocity.y + rand() / (RAND_MAX / velocityDeviation.y) + .5;
	}
	void resetDead(){
		for (Particle &p: particles){
			if (p.position.x == position.x && p.position.y == position.y){
				randomizeAttributes(p);
			}
		}
	}
	
	void Render(ShaderProgram * program){
		vector<float> vertices;
		vector<float> particleColors;
		vector<float> texCoords;
		for (int i = 0; i < particles.size(); i++){
			vertices.push_back(particles[i].position.x);
			vertices.push_back(particles[i].position.y);

			float m = (particles[i].lifetime / maxLifetime);
			float size = lerp(startSize, endSize, m) + particles[i].sizeDeviation;
			//float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			vertices.insert(vertices.end(), {
				particles[i].position.x - size, particles[i].position.y + size,
				particles[i].position.x - size, particles[i].position.y - size,
				particles[i].position.x + size, particles[i].position.y + size,

				particles[i].position.x + size, particles[i].position.y + size,
				particles[i].position.x - size, particles[i].position.y - size,
				particles[i].position.x + size, particles[i].position.y - size
			});
			texCoords.insert(texCoords.end(), {
				0.0f, 0.0f,
				0.0f, 1.0f,
				1.0f, 0.0f,

				1.0f, 0.0f,
				0.0f, 1.0f,
				1.0f, 1.0f
			});
			
			//	float m = (particles[i].lifetime / maxLifetime);
			//	particleColors.push_back(lerp(startColor.r, endColor.r, m));
			//	particleColors.push_back(lerp(startColor.g, endColor.g, m));
			//	particleColors.push_back(lerp(startColor.b, endColor.b, m));
			//	particleColors.push_back(lerp(startColor.a, endColor.a, m));
		}
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size()/2);
	}
	void RenderTexture(ShaderProgram * program){
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		for (int i = 0; i < particles.size(); i++){
			GLfloat vertices[] =
			{
				particles[i].position.x - particles[i].width / 2, particles[i].position.y - particles[i].height / 2,
				particles[i].position.x + particles[i].width / 2, particles[i].position.y - particles[i].height / 2,
				particles[i].position.x + particles[i].width / 2, particles[i].position.y + particles[i].height / 2,
				particles[i].position.x - particles[i].width / 2, particles[i].position.y - particles[i].height / 2,
				particles[i].position.x + particles[i].width / 2, particles[i].position.y + particles[i].height / 2,
				particles[i].position.x - particles[i].width / 2, particles[i].position.y + particles[i].height / 2 //0,0
			};

			program->setModelMatrix(particles[i].matrix);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //not white(?)
//			glBlendFunc(GL_SRC_ALPHA, GL_ONE); //white
			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program->positionAttribute);
			glBindTexture(GL_TEXTURE_2D, particleTexture);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	

};

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Free Chinatsu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	SDL_Event event;
	bool done = false;

	
	glViewport(0, 0, 640, 360);          
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");          

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(program.programID);
	
	Matrix projectionMatrix;     
	Matrix modelMatrix;  
	Matrix modelMatrixc;
	Matrix viewMatrix;     
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);

	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	GLuint font = LoadTexture("font.png");
	
	Entity player;
	player.player();
	player.updateImage(&program, texCoords);
	
	Entity block;
	block.tile();
	block.updateImage(&program, texCoords);

	glUseProgram(program.programID);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	
	float lastFrameTicks = 0.0f;
	
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	float x=0.0f, y=0.0f;
	Matrix startMenu;
	ParticleEmitter fountain(300);
	//viewMatrix.Scale(30, 30, 0);
	bool projectileOn = false;
	float counter = 0;
	while (!done) {

		glClear(GL_COLOR_BUFFER_BIT);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		x += elapsed;
		y += elapsed;

		
		glEnableVertexAttribArray(program.positionAttribute);
		glEnableVertexAttribArray(program.texCoordAttribute);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);
		program.setModelMatrix(startMenu);
		//viewMatrix.setPosition(player.x, player.y, 0.0f);
		//player.updateImage(&program, texCoords);
		//block.updateImage(&program, texCoords);
		Matrix fontText;
		fontText.identity();
		
		/*if (player.collidesWith(&block)){
			DrawText(&program, fontText, font, "True", .2f, 0.2f, -2.0f, -1.0f);
		}
		else{
			DrawText(&program, fontText, font, "False", .2f, 0.2f, -2.0f, -1.0f);
		}
		*/
//		void DrawText(ShaderProgram *program, Matrix& matrix, int fontTexture, string text, float size, float spacing, float x, float y) {

		if (keys[SDL_SCANCODE_UP]){
			fountain.position.y += 2.0f * elapsed;
		}
		if (keys[SDL_SCANCODE_DOWN]){
			fountain.position.y -= 2.0f * elapsed;
		}
		if (keys[SDL_SCANCODE_LEFT]){
			fountain.position.x -= 2.0f * elapsed;
		}
		if (keys[SDL_SCANCODE_RIGHT]){
			fountain.position.x += 2.0f * elapsed;
		}
		/*
		everlasting jizz
		*/
		if (keys[SDL_SCANCODE_SPACE]){
			fountain.Update(elapsed);
			//fountain.Render(&program);
			fountain.RenderTexture(&program);
		}
		
		if (projectileOn == true){
			fountain.Update(elapsed);
			fountain.RenderTexture(&program);
			counter += elapsed;
			if (counter > 3){
				fountain.setStopFlow(false);
			}
		}

		if (keys[SDL_SCANCODE_SPACE]){
			projectileOn = true;
			fountain.setStopFlow(true);
			counter = 0;
		}
	
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

