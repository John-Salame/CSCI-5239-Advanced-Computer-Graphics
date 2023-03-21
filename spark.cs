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
uniform float lifespan;
uniform float deltaTime;
// Weak Gravity
// const vec3 G = vec3(0.0, -3.0, 0.0);
const float resistance = 0.3; // somewhat strong air resistance
// Time step
const float  dt = 0.1;

void main()
{
  //  Global Thread ID
  uint gid = gl_GlobalInvocationID.x;

  //  Get position and velocity
  vec3 p0 = pos[gid].xyz;
  vec3 v0 = vel[gid].xyz;

  // Don't apply gravity
  vec3 p = p0 + v0 * dt;
  // Apply air resistance after moving the point
  vec3 v = v0 * (1 - dt * resistance);

  float lifetimeInstance = lifetime[gid];
  lifetimeInstance += deltaTime;

  pos[gid].xyz = p;
  vel[gid].xyz = v;
  lifetime[gid] = lifetimeInstance;
}
