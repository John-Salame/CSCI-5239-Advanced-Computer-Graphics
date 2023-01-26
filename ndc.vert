// Author: John Salame
// NDC vertex shader
// Goal is to do less than 3 lines
// Color the vertex with RGB corresponding to the Normalized Device Coordinate (NDC)
// Technically it is done in four lines, but it could be condensed down to three lines if I forego clarity.
//   Line 1: Calculate clip coordinates.
//   Line 2: Set color.
//   Line 3: Set position.
// With this setup, the fragment shader is unneeded, so I use NULL for the fragment shader in the shader program.

// It may also be possible to bypass the vertex shader altogether and use only the fragment shader 
// if I pass in the window width and height as uniforms and use gl_FragCoord.
// However, I still do not think this would meet the one-line code challenge.
#version 120

void main() {
  // four-dimensional position
  vec4 clipCoord = gl_ModelViewProjectionMatrix * gl_Vertex;
  // use the ndc as the color
  vec4 ndc = clipCoord / clipCoord.w;
  gl_FrontColor = (ndc + 1.0) / 2.0; // use NDC adjusted so that -1=0 and 1=1
  gl_Position = clipCoord; // this is required or nothing shows up
}
