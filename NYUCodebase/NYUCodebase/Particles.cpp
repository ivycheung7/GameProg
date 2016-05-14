#include "Particles.h";


/*
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
*/

Particles::ParticleEmitter(){
	//Default # of particles is 400
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

	for (int i = 0; i < 400; i++){
		//Random since we dont want particles to reset together
		Particle p;
		randomize(p);
		particles.push_back(p);
	}
}
Particles::ParticleEmitter(unsigned int particleCount){
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

void Particles::setEmitterPosition(float x, float y){
	position.x = x;
	position.y = y;
}

void Particles::setEmitterVelocity(float x, float y){
	velocity.x = x;
	velocity.y = y;
}

void Particles::randomize(Particle& p){
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
void Particles::resetAll(){
		//Resets all particles to be alive and randomizes attributes
		for (Particle p : particles){
			ressurect(p);
			randomize(p);
		}
	}
void Particles::kill(Particle& p){
		//Kills it, and makes the width and height super tiny. If 0, then the render crashes (?)
		p.isDead = true;
		p.width = 0.001;
		p.height = 0.001;
	}
void Particles::ressurrect(Particle& p){
		p.isDead = false;
		p.width = .5f;
		p.height = .5f;

	}
void Particles::setStopFlow(){
		stopFlow = true;
	}
void Particles::Update(float elapsed){
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
			//	if (stopFlow){
			//		kill(particles[i]);
			//	}
				//not sure if particles[i].position = position; does the same job
			}
			else{
				particles[i].position.x += elapsed * particles[i].velocity.x;
				particles[i].position.y += elapsed * particles[i].velocity.y;
				particles[i].velocity.y += -9.8 * elapsed;
			}
		}
	}
void Particles::randomizeAttributes(Particle& p){
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
void Particles::resetDead(){
		for (Particle &p : particles){
			if (p.position.x == position.x && p.position.y == position.y){
				randomizeAttributes(p);
			}
		}
	}

void Particles::Render(ShaderProgram * program){
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

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
	}
void Particles::RenderTexture(ShaderProgram * program){
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