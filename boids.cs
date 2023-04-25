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
// related to steering toward average heading
const float localDistance = 0.5; // how close other members must be in order to be considered "flockmates"
const float localDistanceSquared = localDistance * localDistance;
const float targetSteeringTime = 1.0; // how many seconds it should take to match the average heading of the flockmates
const float minSteerAcceleration = 1.0; // we need this to be larger than min cohesion speed so we focus on steering toward average heading once we are near the flock.
const float maxSteerAcceleration = 1.5;

// related to cohesion
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
  // 2. Steer toward the average heading of local flockmates
  vec3 heading = vec3(0.0);
  float neighbors = 0; // number of neighbors
  for (int i = 0; i < numFireflies; ++i)
  {
    vec3 diff = p0 - pos[i].xyz;
    float dist = dot(diff, diff);
    float close = step(localDistanceSquared, dist); // 1.0 if you are nearby (local flockmates)
    neighbors += close;
    heading += vel[i].xyz * close;
  }
  heading /= neighbors;
  vec3 steeringRequired = heading - v0;
  vec3 steeringAcceleration = steeringRequired / targetSteeringTime;
  float steeringAccelerationMagnitude = length(steeringAcceleration);
  speedLimit = min(steeringAccelerationMagnitude, maxSteerAcceleration) / steeringAccelerationMagnitude;
  steeringAcceleration *= speedLimit;
  speedLimit = max(steeringAccelerationMagnitude, minSteerAcceleration) / steeringAccelerationMagnitude;
  steeringAcceleration *= speedLimit;

  // 3. Calculate center of mass for cohesion
  vec3 com; // com = center of mass
  for (int i = 0; i < numFireflies; ++i)
  {
    com += pos[i].xyz;
  }
  com /= numFireflies;
  vec3 cohesion = com - p0;
  vec3 cohesionVelocity = cohesion / travelTimeToCenter; // note: this is really an acceleration. It's poorly named.
  float cohesionSpeed = length(cohesionVelocity);
  speedLimit = min(cohesionSpeed, maxCohesionSpeed) / cohesionSpeed;
  cohesionVelocity *= speedLimit;
  speedLimit = max(cohesionSpeed, minCohesionSpeed) / cohesionSpeed;
  cohesionVelocity *= speedLimit;

  vec3 v = v0 + (steeringAcceleration + cohesionVelocity) * dt;
  vec3 p = p0 + v * dt;

  pos[gid].xyz = p;
  vel[gid].xyz = v;
}
