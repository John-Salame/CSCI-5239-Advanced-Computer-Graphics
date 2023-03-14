// John Salame
// Place particles using only position and color for now.
// This is useful for axes and light sources
#version 400 core

uniform mat4 ViewMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;

in vec4 Vertex;
in vec4 Color;
out vec4 FrontColor;

void main() {
  gl_Position = ModelViewProjectionMatrix * Vertex;
  FrontColor = Color;
}