// John Salame
// Particle fragment shader -- blends texture lookup with input color
// Adapted from fire.frag in Example 16
#version 400 core

uniform sampler2D sprite;
in vec4 FrontColor;
out vec4 FragColor;

void main()
{
   //  Sample texture
   vec4 color = texture2D(sprite,gl_PointCoord.st);
   //  Blend point color with texture. Rely on the color's alpha.
   FragColor = FrontColor*color;
}