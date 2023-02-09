// John Salame
// Polka Dots shader-generated texture
#version 120

uniform float dim;
varying vec4 intensity;

void main() {
  vec2 p = gl_TexCoord[0].xy;
  vec2 r = mod(p, 2*dim);
  r.x = r.x>dim? r.x-2*dim : r.x; // allow for negative offsets from the center
  r.y = r.y>dim? r.y-2*dim : r.y; // allow for negative offsets from the center
  // circle with radius 0.8*dim
  float eqn = 0.8*dim - length(r);
  if(eqn < 0) {
    // yellow outside the shape
    gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0) * intensity;
  }
  else {
    // purple inside shape
    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0) * intensity;
  }
}