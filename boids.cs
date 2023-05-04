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
const float simulationSpeed = 2.0;

// related to separation (avoiding peers who are too close)
const float hostileDistance = 0.2; // change to 0.1 for tighter clusters, more hivemind feel
const float hostileDistanceSquared = hostileDistance * hostileDistance;
const float aversionStrength = 2.0; // how much you want to avoid peers. This should be the strongest force if we don't want a super stable flock.
const float aversionDecay = 2.5; // larger aversionDecay increases the possible force closer than hostile distance and shortens the distance where there is no aversion; seems to make the group more malleable in shape.

// related to steering toward average heading
const float localDistance = 0.3; // how close other members must be in order to be considered "flockmates"
const float localDistanceSquared = localDistance * localDistance;
const float targetSteeringTime = 1.5; // how many seconds it should take to match the average heading of the flockmates
const float minSteerAcceleration = 0.2; // 1.0; // we need this to be larger than min cohesion speed so we focus on steering toward average heading once we are near the flock.
const float maxSteerAcceleration = 1.5;

// related to cohesion
const float cohesionLocalDistance = 0.5; // 0.5
const float cohesionLocalDistanceSquared = cohesionLocalDistance * cohesionLocalDistance;
const float travelTimeToCenter = 1.0; // how many seconds you would like to take when trying to reach the center
const float minCohesionSpeed = 0.0; //0.5; // units per second
const float maxCohesionSpeed = 3.0; // units per second

// barrier to keep fireflies from escaping the scene
const float barrierEdge = 4.0; // radius of the "bounding box" (or sphere)
const float barrierLowerEdge = 0.7; // lowest height the fireflies can go to
const float barrierStrength = 1.0; // formerly 0.01


/* Bound a velocity (or vector in general) between the provided speed limits */
void bound(inout vec3 v, in float minV, in float maxV)
{
  float vMag = length(v) + 0.0001; // prevent divide by 0
  float speedLimit = min(vMag, maxV) / vMag; // upper bound
  v *= speedLimit;
  speedLimit = max(vMag, minV) / vMag;
  v *= speedLimit;
}

void main()
{
  //  Global Thread ID
  uint gid = gl_GlobalInvocationID.x;
  float dt = simulationSpeed * deltaTime; // simulation speed * time passed

  // initial positions
  vec3 p0 = pos[gid].xyz;
  vec3 v0 = vel[gid].xyz;
  float speedLimit = 0.0; // use later with a differnt value
  float neighbors = 0.0; // use for separation and alignment.

  // https://en.wikipedia.org/wiki/Boids
  // 1. Separation: Avoid crowding local flockmates
  vec3 repulsion = vec3(0.0);
  for (int i = 0; i < numFireflies; ++i)
  {
    vec3 diff = p0 - pos[i].xyz;
    float dist = dot(diff, diff); // this is actually the squared distance
    float close = step(dist, 2.0 * hostileDistanceSquared); // 1.0 if you are nearby (local flockmates)
    neighbors += close;
    // this time I'll try a non-local aversion with strength <aversionStrength> at the hostile distance
    repulsion += normalize(diff) * aversionStrength * max(0.0, 1.0 + aversionDecay * (hostileDistanceSquared - dist) / hostileDistanceSquared);
  }
  repulsion /= neighbors;

  // 2. Alignment: Steer toward the average heading of local flockmates
  vec3 heading = vec3(0.0);
  neighbors = 0.0; // number of neighbors
  for (int i = 0; i < numFireflies; ++i)
  {
    vec3 diff = p0 - pos[i].xyz;
    float dist = dot(diff, diff);
    float close = step(dist, localDistanceSquared); // 1.0 if you are nearby (local flockmates)
    neighbors += close;
    heading += vel[i].xyz * close;
  }
  heading /= neighbors;
  vec3 steeringRequired = heading - v0;
  vec3 steeringAcceleration = steeringRequired / targetSteeringTime;
  bound(steeringAcceleration, minSteerAcceleration, maxSteerAcceleration);

  // 3. Cohesion: Calculate center of mass for cohesion
  vec3 com = vec3(0.0); // com = center of mass in the local cluster
  neighbors = 0.0;
  for (int i = 0; i < numFireflies; ++i)
  {
    // determine if you are near point i
    vec3 diff = p0 - pos[i].xyz;
    float dist = dot(diff, diff);
    float close = step(dist, cohesionLocalDistanceSquared); // 1.0 if you are nearby (local flockmates)
    neighbors += close;
    // only use points close to you in center of mass calculation (local center of mass)
    com += pos[i].xyz * close;
  }
  com /= neighbors;
  vec3 cohesion = com - p0;
  vec3 cohesionVelocity = cohesion / travelTimeToCenter; // note: this is really an acceleration. It's poorly named.
  bound(cohesionVelocity, minCohesionSpeed, maxCohesionSpeed);

  // barrier to prevent excape from scene
  vec3 barrierDir = vec3(0, 1.5, 0) - p0;
  float applyBarrierForce = step(barrierEdge, length(barrierDir));
  vec3 barrierAcceleration = applyBarrierForce * normalize(barrierDir) * barrierStrength;
  // apply upward acceleration if you are too low
  float riseUp = step(p0.y, barrierLowerEdge);
  barrierAcceleration += riseUp * vec3(0.0, 1.0, 0.0);

  vec3 v = v0 + (repulsion + steeringAcceleration + cohesionVelocity + barrierAcceleration) * dt;
  vec3 p = p0 + v * dt;

  pos[gid].xyz = p;
  vel[gid].xyz = v;
}
