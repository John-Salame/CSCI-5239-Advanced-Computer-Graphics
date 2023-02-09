// John Salame
// I originally tried to make a cardioid, but it didn't work out.
// After a few attempts, I ended up making a sawtooth pattern by accident.
#version 120

uniform float dim;
varying vec4 intensity;

void main() {
  vec2 p = gl_TexCoord[0].xy;
  vec2 r = mod(p, 2*dim);
  r.x = r.x>dim? r.x-2*dim : r.x; // allow for negative offsets from the center
  r.y = r.y>dim? r.y-2*dim : r.y; // allow for negative offsets from the center
  float th = atan(r.y, r.x);
  float eqn = r.x/dim - (1 - cos(th));
  if(eqn < 0) {
    // green outside the shape
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0) * intensity;
  }
  else {
    // red inside shape
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0) * intensity;
  }
}
