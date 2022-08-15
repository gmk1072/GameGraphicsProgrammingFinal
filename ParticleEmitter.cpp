#include "ParticleEmitter.h"
#include "MemoryDebug.h"

// --------------------------------------------------------
// Constructor
//
// Creates a burst particle emitter.
//
// numParticles - Number of particles to emit every time 
//				  Emit is called on this particle emitter.
// --------------------------------------------------------
ParticleEmitter::ParticleEmitter(unsigned int numParticles) :
	type(ParticleEmitterType::BURST),
	counter(0),
	numLoops(1),
	currNumLoops(1),
	numParticles(numParticles),
	isActive(false),
	isLoopable(false)
{
	// Zero out structs to ensure we're starting fresh.
	memset(&emitter, 0, sizeof(Emitter));

	// Find the closest aligned power for 2 for dispatching compute shaders
	numParticlesAligned = (numParticles - 1 + NUM_PARTICLE_THREADS) & (~(NUM_PARTICLE_THREADS - 1));
}

// --------------------------------------------------------
// Constructor
//
// Creates a continuous particle emitter.
//
// particlesPerRate - Amount of particles to emit every
//					  "rate" seconds.
// rate - Amount of seconds that need to pass before emitting
//		  "particlesPerRate" particles.
// --------------------------------------------------------
ParticleEmitter::ParticleEmitter(unsigned int particlesPerRate, float rate) :
	type(ParticleEmitterType::CONTINOUS),
	counter(0),
	numLoops(1),
	currNumLoops(1),
	numParticles(particlesPerRate),
	emitRate(rate),
	isActive(false),
	isLoopable(true)
{
	// Zero out structs to ensure we're starting fresh.
	memset(&emitter, 0, sizeof(Emitter));

	// Find the closest aligned power for 2 for dispatching compute shaders
	numParticlesAligned = (numParticles - 1 + NUM_PARTICLE_THREADS) & (~(NUM_PARTICLE_THREADS - 1));
}

// --------------------------------------------------------
// Destructor
// --------------------------------------------------------
ParticleEmitter::~ParticleEmitter()
{
}

// --------------------------------------------------------
// Set initial tint of particles.
//
// initialTint - RGB value from 0 to 1.
// --------------------------------------------------------
void ParticleEmitter::SetTint(DirectX::XMFLOAT3 & initialTint)
{
	emitter.iMinTint = initialTint;
	emitter.flags &= (~RAND_ITINT);
}

// --------------------------------------------------------
// Set end tint of particles.
//
// NOTE: For this to do anything, be sure to SetInterpTint(true).
//
// initialTint - RGB value from 0 to 1.
// --------------------------------------------------------
void ParticleEmitter::SetEndTint(DirectX::XMFLOAT3 & endTint)
{
	emitter.eMinTint = endTint;
	emitter.flags &= (~RAND_ETINT);
}

// --------------------------------------------------------
// Set initial speed of particles.
//
// initialSpeed - Initial speed.
// --------------------------------------------------------
void ParticleEmitter::SetSpeed(float initialSpeed)
{
	emitter.iMinSpeed = initialSpeed;
	emitter.flags &= (~RAND_ISPEED);
}

// --------------------------------------------------------
// Set end speed of particles.
//
// NOTE: For this to do anything, be sure to SetInterpSpeed(true).
//
// initialSpeed - End speed.
// --------------------------------------------------------
void ParticleEmitter::SetEndSpeed(float endSpeed)
{
	emitter.eMinSpeed = endSpeed;
	emitter.flags &= (~RAND_ESPEED);
}

// --------------------------------------------------------
// Set initial size of particles.
//
// initialSize - Initial size.
// --------------------------------------------------------
void ParticleEmitter::SetSize(DirectX::XMFLOAT2 & initialSize)
{
	emitter.iMinSize = initialSize;
	emitter.flags &= (~RAND_ISIZE);
}

// --------------------------------------------------------
// Set end size of particles.
//
// NOTE: For this to do anything, be sure to SetInterpSize(true).
//
// endSize - End size.
// --------------------------------------------------------
void ParticleEmitter::SetEndSize(DirectX::XMFLOAT2 & endSize)
{
	emitter.eMinSize = endSize;
	emitter.flags &= (~RAND_ESIZE);
}

// --------------------------------------------------------
// Set alpha value of particles.
//
// NOTE: Making this anything but 1.0f in deferred rendering
//		 will look pretty bad.
//
// initialAlpha - Initial alpha of particles.
// --------------------------------------------------------
void ParticleEmitter::SetAlpha(float initialAlpha)
{
	emitter.iAlpha = initialAlpha;
}

// --------------------------------------------------------
// Set end alpha value of particles.
//
// NOTE: For this to do anything, be sure to SetInterpAlpha(true).
// NOTE: Making this anything but 1.0f in deferred rendering
//		 will look pretty bad.
//
// endAlpha - End alpha of particles.
// --------------------------------------------------------
void ParticleEmitter::SetEndAlpha(float endAlpha)
{
	emitter.eAlpha = endAlpha;
}

// --------------------------------------------------------
// Set direction of particles.
//
// dir - XYZ unit vector.
// --------------------------------------------------------
void ParticleEmitter::SetDirection(DirectX::XMFLOAT3 & dir)
{
	emitter.minXYZTheta = dir;
	emitter.flags &= (~RAND_DIR);
}

// --------------------------------------------------------
// Set particle age.
//
// age - Amount of time before particles die. Ranged values
//		 and other variables that have a "SetInterp..."
//		 function will interpolate over age.
// --------------------------------------------------------
void ParticleEmitter::SetAge(float age)
{
	emitter.minAge = age;
	emitter.flags &= (~RAND_AGE);
}

// --------------------------------------------------------
// Set emitter position. This will also be each particles
// initial position.
//
// pos - Position of emitter.
// --------------------------------------------------------
void ParticleEmitter::SetPosition(const DirectX::XMFLOAT3 & pos)
{
	emitter.position = pos;
}

// --------------------------------------------------------
// Set seed or "nonce" for this particle emitter. Should be
// changed every frame to a new random value. Will be used
// to calculate pseudo random numbers in compute shaders.
//
// nonce - Nonce for this emitter.
// --------------------------------------------------------
void ParticleEmitter::SetNonce(unsigned int nonce)
{
	emitter.nonce = nonce;
}

// --------------------------------------------------------
// Set the texture ID of the texture to use for this particle
// within the particle renderer's texture atlas.
//
// textureID - Texture ID to use for all particles spawning
//			   from this emitter.
//
// returns - True if the value was set successfully.
// --------------------------------------------------------
bool ParticleEmitter::SetTextureID(unsigned int textureID)
{
	// If larger then 1 << TEXT_ID_SIZE, wont fit in options
	if (textureID > ((1 << (TEXT_ID_SIZE + 1)) - 1))
		return false;
	
	// Create mask to preserve lower 12 bits
	unsigned int mask = 0xFFF;

	// And with mask to wipe out top 20
	emitter.flags &= mask;

	// Bit shift texture ID to left 12 bits
	textureID <<= ((sizeof(unsigned int) * 8) - TEXT_ID_SIZE);

	// Or with flags (done...)
	emitter.flags |= textureID;
	return true;
}

// --------------------------------------------------------
// Set the initial tint range.
//
// NOTE: A randomly interpolated color will be selected
//		 within the provided range at emit time.
//
// initialMinTint - Initial minimum tint.
// initialMaxTint - Initial maximum tint.
// --------------------------------------------------------
void ParticleEmitter::SetInitialTintRange(DirectX::XMFLOAT3 & initialMinTint, DirectX::XMFLOAT3 & initialMaxTint)
{
	emitter.iMinTint = initialMinTint;
	emitter.iMaxTint = initialMaxTint;
	emitter.flags |= RAND_ITINT;
}

// --------------------------------------------------------
// Set the end tint range.
//
// NOTE: A randomly interpolated color will be selected
//		 within the provided range at emit time.
//
// endMinTint - End minimum tint.
// endMaxTint - End maximum tint.
// --------------------------------------------------------
void ParticleEmitter::SetEndTintRange(DirectX::XMFLOAT3 & endMinTint, DirectX::XMFLOAT3 & endMaxTint)
{
	emitter.eMinTint = endMinTint;
	emitter.eMaxTint = endMaxTint;
	emitter.flags |= RAND_ETINT;
}

// --------------------------------------------------------
// Set the initial speed range.
//
// NOTE: A random number will be selected within the provided
//		 range at emit time.
//
// initialMinSpeed - Initial minimum speed.
// initialMaxSpeed - Initial maximum speed.
// --------------------------------------------------------
void ParticleEmitter::SetInitialSpeedRange(float initialMinSpeed, float initialMaxSpeed)
{
	emitter.iMinSpeed = initialMinSpeed;
	emitter.iMaxSpeed = initialMaxSpeed;
	emitter.flags |= RAND_ISPEED;
}

// --------------------------------------------------------
// Set the end speed range.
//
// NOTE: A random number will be selected within the provided
//		 range at emit time.
//
// endMinSpeed - End minimum speed.
// endMaxSpeed - End maximum speed.
// --------------------------------------------------------
void ParticleEmitter::SetEndSpeedRange(float endMinSpeed, float endMaxSpeed)
{
	emitter.eMinSpeed = endMinSpeed;
	emitter.eMaxSpeed = endMaxSpeed;
	emitter.flags |= RAND_ESPEED;
}

// --------------------------------------------------------
// Set the initial size range.
//
// NOTE: A randomly interpolated size will be selected
//		 within the provided range at emit time.
//
// initialMinSize - Initial minimum size.
// initialMaxSize - Initial maximum size.
// --------------------------------------------------------
void ParticleEmitter::SetInitialSizeRange(DirectX::XMFLOAT2 & initialMinSize, DirectX::XMFLOAT2 & initialMaxSize)
{
	emitter.iMinSize = initialMinSize;
	emitter.iMaxSize = initialMaxSize;
	emitter.flags |= RAND_ISIZE;
}

// --------------------------------------------------------
// Set the end size range.
//
// NOTE: A randomly interpolated size will be selected
//		 within the provided range at emit time.
//
// endMinSize - End minimum size.
// endMaxSize - End maximum size.
// --------------------------------------------------------
void ParticleEmitter::SetEndSizeRange(DirectX::XMFLOAT2 & endMinSize, DirectX::XMFLOAT2 & endMaxSize)
{
	emitter.eMinSize = endMinSize;
	emitter.eMaxSize = endMaxSize;
	emitter.flags |= RAND_ESIZE;
}

// --------------------------------------------------------
// Set the direction range.
//
// NOTE: Component-wise random numbers will be generated
//		 within the provided range at emit time.
//
// minDir - Minimum direction (unit vector XYZ).
// maxDir - Maximum direction (unit vector XYZ).
// --------------------------------------------------------
void ParticleEmitter::SetDirectionRange(DirectX::XMFLOAT3 & minDir, DirectX::XMFLOAT3 & maxDir)
{
	emitter.minXYZTheta = minDir;
	emitter.maxXYZTheta = maxDir;
	emitter.flags |= RAND_DIR;
}

// --------------------------------------------------------
// Set the age range.
//
// NOTE: A random number will be selected within the provided
//		 range at emit time.
// NOTE: Ranged and other variables that have a "SetInterp..."
//		 function will interpolate over age.
//
// minAge - Minimum age.
// maxAge - Maximum age.
// --------------------------------------------------------
void ParticleEmitter::SetAgeRange(float minAge, float maxAge)
{
	emitter.minAge = minAge;
	emitter.maxAge = maxAge;
	emitter.flags |= RAND_AGE;
}

// --------------------------------------------------------
// Enable or disable interpolation of alpha over lifetime.
//
// NOTE: Initial and end values must be set for this to do
//		 anything.
//
// val - True to interpolate over this value.
// --------------------------------------------------------
void ParticleEmitter::SetInterpAlpha(bool val)
{
	if(val)
		emitter.flags |= INTERP_ALPHA;
	else
		emitter.flags &= (~INTERP_ALPHA);
}

// --------------------------------------------------------
// Enable or disable interpolation of speed over lifetime.
//
// NOTE: Initial and end values must be set for this to do
//		 anything.
//
// val - True to interpolate over this value.
// --------------------------------------------------------
void ParticleEmitter::SetInterpSpeed(bool val)
{
	if (val)
		emitter.flags |= INTERP_SPEED;
	else
		emitter.flags &= (~INTERP_SPEED);
}

// --------------------------------------------------------
// Enable or disable interpolation of size over lifetime.
//
// NOTE: Initial and end values must be set for this to do
//		 anything.
//
// val - True to interpolate over this value.
// --------------------------------------------------------
void ParticleEmitter::SetInterpSize(bool val)
{
	if (val)
		emitter.flags |= INTERP_SIZE;
	else
		emitter.flags &= (~INTERP_SIZE);
}

// --------------------------------------------------------
// Enable or disable interpolation of tint over lifetime.
//
// NOTE: Initial and end values must be set for this to do
//		 anything.
//
// val - True to interpolate over this value.
// --------------------------------------------------------
void ParticleEmitter::SetInterpTint(bool val)
{
	if (val)
		emitter.flags |= INTERP_TINT;
	else
		emitter.flags &= (~INTERP_TINT);
}

// --------------------------------------------------------
// Set the number of times this emitter should emit.
//
// NOTE: Does not apply to burst particle emitters.
//
// loopAmount - Number of times to loop emission.
//				-1 for infinite loop (default).
//				0 to stop emission.
// --------------------------------------------------------
void ParticleEmitter::SetLoop(int loopAmount = -1)
{
	numLoops = loopAmount;
	currNumLoops = numLoops;
}

// --------------------------------------------------------
// Emits particles.
// --------------------------------------------------------
void ParticleEmitter::Emit()
{
	// If already active, do nothing
	if (isActive)
		return;

	// Set active
	isActive = true;
	
	// Reset loop count
	currNumLoops = numLoops;
}

// --------------------------------------------------------
// Check to see if this particle emitter can emit this frame.
//
// dt - Delta time.
//
// return - True if emitter is ready to emit.
// --------------------------------------------------------
bool ParticleEmitter::CanEmit(float dt)
{
	if (isActive)
	{
		// Decrease counter
		counter -= dt;
		if (counter < 0.0f)
		{
			emitter.numToEmit = numParticles;
			switch (type)
			{
			case ParticleEmitterType::BURST:
				// Based on max possible lifetime
				counter = DirectX::XMMax<float>(emitter.minAge, emitter.maxAge);
				break;
			case ParticleEmitterType::CONTINOUS:
				// Based on emit rat provided
				counter = emitRate;
				break;
			default:
				break;
			}

			if (currNumLoops >= 0 && currNumLoops-- == 0)
			{
				isActive = false;
				counter = 0;
			}

			return isActive;
		}
		return false;
	}
	return false;
}