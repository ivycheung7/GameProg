
#include <vector>
#include <math.h>
#include <SDL_mixer.h>

struct Pair{
	float x;
	float y;
};

struct Particle{
	Pair position;
	Pair velocity;
	float lifetime;

	float width = .5f;
	float height = .5f;
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
	ParticleEmitter();
	ParticleEmitter(unsigned int particleCount);
	//ParticleEmitter();
	//~ParticleEmitter();

	void randomize(Particle& p);
	void resetAll();
	void kill(Particle& p);
	void ressurrect(Particle& p);
	void setStopFlow();
	void Update(float elapsed);
	void randomizeAttributes(Particle& p);
	void resetDead();

	void Render(ShaderProgram * program);
	void RenderTexture(ShaderProgram * program);
};