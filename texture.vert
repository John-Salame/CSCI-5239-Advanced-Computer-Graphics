// John Salame
// Place vertices and set texture coordinates
// This is useful for axes and light sources
#version 400 core

uniform mat4 ViewMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;

in vec3 Vertex;
in vec2 Texture;
out vec4 FrontColor;
out vec2 Texcoord;

void main() {
  gl_Position = ModelViewProjectionMatrix * vec4(Vertex, 1.0);
  FrontColor = vec4(1.0);
  Texcoord = Texture;
}
