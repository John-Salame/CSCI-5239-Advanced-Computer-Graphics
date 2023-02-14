// John Salame
// Simply place the vertices -- do not do lighting or textures
// This is useful for axes and light sources
#version 400 core

uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;
out vec4 FrontColor;

void main() {
  gl_Position = ModelViewProjectionMatrix * gl_Vertex;
  FrontColor = vec4(1.0);
}
