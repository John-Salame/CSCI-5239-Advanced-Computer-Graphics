// Author: John Salame
// NDC fragment shader
// Color the vertex with RGB corresponding to the Normalized Device Coordinate (NDC)
#version 120

void main()
{
  gl_FragColor = gl_Color;
  // gl_FragColor = gl_FragCoord;
}