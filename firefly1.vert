// John Salame
// Light objects using fireflies as point lights with attenuation and range
// Lighting does not account for normals
#version 400 core

uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;

//  Vertex attributes (input)
in vec4 Vertex;
in vec3 Normal;
in vec4 Color;
in vec2 Texture;

out vec4 FrontColor;

const float squaredRange = 0.25;

void main() {
  vec4 pos = ModelViewMatrix * Vertex;
  vec4 color = Color;
  // color grass more yellow if close to a firefly
  for(int i = 0; i < 4; i++) {
    vec4 fireflyPos = gl_LightSource[i].position;
    vec3 diff = pos.xyz - fireflyPos.xyz;
    // color = vec4(diff, 1.0);
    float mag = dot(diff, diff);
    if(mag < squaredRange) {
      color += vec4(0.1, 0.05, 0.0, 1.0) / (mag+0.1);
    }
  }
  gl_Position = ModelViewProjectionMatrix * Vertex;
  FrontColor = color;
}