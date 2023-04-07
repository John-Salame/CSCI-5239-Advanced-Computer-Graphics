//
// John Salame
// Compute Shader for sparks coming from fireflies
// Some physics and other code based on Example 19
// Note: My computer does not support the GL_ARB_compute_variable_group_size extension.
//

#version 440 compatibility

//  Array positions
layout(binding = 4) buffer posbuf { vec4 pos []; };
layout(binding = 5) buffer velbuf { vec4 vel []; };
layout(binding = 6) buffer lifetimebuf { float lifetime []; };
//  Work group size
layout(local_size_x = 100, local_size_y = 1, local_size_z = 1) in; // 100 threads per work group

// Uniforms
uniform float lifespan; // max lifetime
uniform float deltaTime;
uniform float t;
uniform sampler3D noiseTexture;

// Constants
const vec3 G = vec3(0.0, -3.0, 0.0); // weak gravity
const float resistance = 0.5; // somewhat strong air resistance
const float simulationSpeed = 1.0;
const vec3 minPosition = vec3(-0.15, -0.15, -0.15);
const vec3 maxPosition = vec3(0.15, 0.15, 0.15);
const vec3 minVelocity = vec3(-2, -2, -2);
const vec3 maxVelocity = vec3(2, 4, 2);


// produce a vec3 with each element between the min and max using a distribution created by Perlin Noise
vec3 myRand(vec3 seed, vec3 randMin, vec3 randMax)
{
  vec4 randResult = texture(noiseTexture, seed);
  vec3 mixPercent = randResult.xyz * vec3(2.0, 4.0, 8.0);
  return randMin + (randMax - randMin) * mixPercent;
}

// produce random numbers favoring the extremes, with a cutoff preventing mid-range numbers from existing
// enter a cutoff value of 0 to generate only numbers at the min or max, and a cutoff of 1 to allow all numbers, and an intermediate cutoff for an intermediate effect.
// assume randMax >= randMin (I don't know what would happen otherwise)
vec3 extremeRand(vec3 seed, float cutoff, vec3 randMin, vec3 randMax)
{
  vec3 randResult = myRand(seed, randMin, randMax);
  vec3 midpoint = (randMax + randMin) / 2.0;
  vec3 span = (randMax - randMin) / 2.0; // dist from midpoint to extreme
  vec3 sign = (2.0 * step(0, randResult - midpoint)) - 1.0; // 0 counts as positive. Sign will be 1 or -1.
  vec3 bounds = span * cutoff; // maximum distance of a random value from an endpoint
  vec3 distFromEndpoint = abs(randResult - midpoint);
  vec3 adjustment = bounds * (distFromEndpoint * distFromEndpoint) / (span * span + 1); // element-wise operations
  return midpoint + sign * (span - adjustment);
}

void resetPositionWhenNecessary(in float seed, float tolerance, in float lifespan, in float timeSinceBirth, in vec3 position, in vec3 velocity, out vec3 newPos, out vec3 newVel, out float newTimeSinceBirth)
{
  float dead = 1 - step(timeSinceBirth, lifespan); // 0.0 if timeSinceBirth <= lifespan, 1.0 if timeSinceBirth > lifespan
  // if particle is "dead," re-initialize position and velocity. Otherwise, leave them be (left part would be 1.0 * existing, right part would be 0.0 * new)
  newPos = mix(position, extremeRand(position + seed, 0.5, minPosition, maxPosition), dead);
  newVel = mix(velocity, extremeRand(velocity + seed, 0.5, minVelocity, maxVelocity), dead);
  newTimeSinceBirth = (1 - dead) * timeSinceBirth; // set newTimeSinceBirth to 0 if dead; keep it unmodified otherwise.
}



void main()
{
  //  Global Thread ID
  uint gid = gl_GlobalInvocationID.x;

  // Time step
  float dt = simulationSpeed * deltaTime; // simulation speed * time passed
  float lifetimeInstance = lifetime[gid];
  lifetimeInstance += deltaTime;

  //  Get position and velocity
  vec3 p0, v0;
  resetPositionWhenNecessary(gid / 100.0 + t, 0.01, lifespan, lifetimeInstance, pos[gid].xyz, vel[gid].xyz, p0, v0, lifetimeInstance); // if the lifetime is over, re-initialize the particle properties

  vec3 p = p0 + v0 * dt;
  // apply gravity (use a minus if you want to invert gravity; it looks cool)
  vec3 v = v0 - G * dt;
  // Apply air resistance after moving the point
  v = v * (1.0 - dt * resistance); // need to use dt somehow to make it consistent no matter the fps

  pos[gid].xyz = p;
  vel[gid].xyz = v;
  lifetime[gid] = lifetimeInstance;
}
