// John Salame
// Simple shader -- do not do lighting or textures
// This is useful for axes and light sources
#version 400 core

in vec4 FrontColor;
out vec4 FragColor;

void main() {
  FragColor = FrontColor;
}
