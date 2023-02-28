// John Salame
// Modified from Example 8
// Blur (low-pass)
//   1 2 1
//   2 1 2   / 13
//   1 2 1
#version 400 core

uniform float dX;
uniform float dY;
uniform sampler2D img;

in vec2 Texcoord;
out vec4 FragColor;

vec4 sample_fun(float dx, float dy)
{
   return texture(img,Texcoord.st + vec2(dx,dy));
}

void main()
{
   float one = 1.0/13.0;
   float two = 2.0/13.0;
   FragColor = one*sample_fun(-dX,+dY) + two*sample_fun(0.0,+dY) + one*sample_fun(+dX,+dY)
                + two*sample_fun(-dX,0.0) + one*sample_fun(0.0,0.0) + two*sample_fun(+dX,0.0)
                + one*sample_fun(-dX,-dY) + two*sample_fun(0.0,-dY) + one*sample_fun(+dX,-dY);
}
