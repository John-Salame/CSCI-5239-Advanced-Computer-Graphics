// John Salame
// Modified from Example 8
// Prewitt edge detection
//    -1 -1 -1       1 0 -1
// H = 0  0  0   V = 1 0 -1
//     1  1  1       1 0 -1
//
// result = sqrt(H^2 + V^2)
#version 400 core

uniform float dX;
uniform float dY;
uniform sampler2D img;

in vec4 FrontColor;
in vec2 Texcoord;
out vec4 FragColor;

vec4 sample_fun(float dx, float dy)
{
   return texture(img,Texcoord.st + vec2(dx,dy));
}

void main()
{
   vec4 H = -sample_fun(-dX,+dY) - sample_fun(0.0,+dY) - sample_fun(+dX,+dY)
            +sample_fun(-dX,-dY) + sample_fun(0.0,-dY) + sample_fun(+dX,-dY);

   vec4 V = sample_fun(-dX,+dY)  - sample_fun(+dX,+dY)
          + sample_fun(-dX,0.0)  - sample_fun(+dX,0.0)
          + sample_fun(-dX,-dY)  - sample_fun(+dX,-dY);

   FragColor = sqrt(H*H+V*V);
}
