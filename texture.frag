// John Salame
// Texture shader -- multiply color and texture component-wise
// This is useful for axes and light sources
#version 400 core

uniform sampler2D tex;
in vec4 FrontColor;
in vec2 Texcoord;
out vec4 FragColor;

void main() {
  FragColor = FrontColor * texture(tex,Texcoord);
}
