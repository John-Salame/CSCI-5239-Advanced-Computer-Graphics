//
// John Salame
// Movement for firefly swarms
// Note: My computer does not support the GL_ARB_compute_variable_group_size extension.
//

#version 440 compatibility

//  Array positions
layout(binding = 4) buffer posbuf { vec4 pos []; };
layout(binding = 5) buffer velbuf { vec4 vel []; };
//  Work group size
layout(local_size_x = 100, local_size_y = 1, local_size_z = 1) in; // 100 threads per work group


// Uniforms
uniform float deltaTime;
uniform int numFireflies;

// Constants
const float simulationSpeed = 1.0;
const float travelTimeToCenter = 2.0; // how many seconds you would like to take when trying to reach the center
const float minCohesionSpeed = 0.5; // units per second
const float maxCohesionSpeed = 2.0; // units per second

void main()
{
  //  Global Thread ID
  uint gid = gl_GlobalInvocationID.x;
  float dt = simulationSpeed * deltaTime; // simulation speed * time passed

  // initial positions
  vec3 p0 = pos[gid].xyz;
  vec3 v0 = vel[gid].xyz;
  float speedLimit = 0.0; // use later with a differnt value

  // https://en.wikipedia.org/wiki/Boids
  // 3. Calculate center of mass for cohesion
  vec3 com;
  for (int i = 0; i < numFireflies; ++i)
  {
    com += pos[i].xyz;
  }
  com /= numFireflies;
  vec3 cohesion = com - p0;
  vec3 cohesionVelocity = cohesion / travelTimeToCenter;
  float cohesionSpeed = length(cohesionVelocity);
  speedLimit = min(cohesionSpeed, maxCohesionSpeed) / cohesionSpeed;
  cohesionVelocity *= speedLimit;
  speedLimit = max(cohesionSpeed, minCohesionSpeed) / cohesionSpeed;
  cohesionVelocity *= speedLimit;

  vec3 v = v0 + cohesionVelocity * dt;
  vec3 p = p0 + v * dt;

  pos[gid].xyz = p;
  vel[gid].xyz = v;
}
