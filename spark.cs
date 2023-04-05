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
uniform sampler3D noiseTexture;

// Constants
// const vec3 G = vec3(0.0, -3.0, 0.0); // weak gravity
const float resistance = 0.05; // somewhat strong air resistance
const float simulationSpeed = 5.0;
const vec3 minPosition = vec3(-0.05, -0.05, -0.05);
const vec3 maxPosition = vec3(0.05, 0.05, 0.05);
const vec3 minVelocity = vec3(-0.3, -0.2, -0.3);
const vec3 maxVelocity = vec3(0.3, 0.4, 0.3);


// produce a vec3 with each element between the min and max
vec3 myRand(vec3 seed, vec3 min, vec3 max)
{
  return abs(texture(noiseTexture, seed)).xyz * (max - min) + min; //element-wise multiplication
  // return 0.5 * (max - min) * (1 + texture(noiseTexture, seed).xyz);
}

void resetPositionWhenNecessary(in float lifespan, in float timeSinceBirth, in vec3 position, in vec3 velocity, out vec3 newPos, out vec3 newVel, out float newTimeSinceBirth)
{
  float posModifier = step(timeSinceBirth, lifespan); // 0 if timeSinceBirth > lifespan, 1.0 otherwise
  // if posModifier is zero, re-initialize position and velocity. Otherwise, leave them be (left part would be 1.0 * existing, right part would be 0.0 * new)
  newPos = posModifier * position + (1 - posModifier) * myRand(position, minPosition, maxPosition);
  newVel = posModifier * velocity + (1 - posModifier) * myRand(velocity, minVelocity, maxVelocity);
  newTimeSinceBirth = posModifier * timeSinceBirth;
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
  resetPositionWhenNecessary(lifespan, lifetimeInstance, pos[gid].xyz, vel[gid].xyz, p0, v0, lifetimeInstance); // if the lifetime is over, re-initialize the particle properties

  // Don't apply gravity
  vec3 p = p0 + v0 * dt;
  // Apply air resistance after moving the point
  vec3 v = v0 * (1.0 - dt * resistance);

  pos[gid].xyz = p;
  vel[gid].xyz = v;
  lifetime[gid] = lifetimeInstance;
}
