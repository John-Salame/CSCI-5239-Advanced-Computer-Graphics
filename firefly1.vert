// John Salame
// CSCI 5239 Advanced Computer Graphics
// Light vertices near the fireflies with a yellow color (uses if statement with max squared radius).
// Light intensity attenuates linearly with distance.
#version 120

void main() {
  vec4 pos = gl_ModelViewMatrix * gl_Vertex;
  vec4 color = gl_Color;
  // color grass more yellow if close to a firefly
  for(int i = 0; i < 4; i++) {
    vec4 fireflyPos = gl_LightSource[i].position;
    vec3 diff = pos.xyz - fireflyPos.xyz;
    // color = vec4(diff, 1.0);
    float mag = dot(diff, diff);
    if(mag < 0.25) {
      color += vec4(0.1, 0.05, 0.0, 1.0) / (mag+0.1);
    }
  }
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_FrontColor = color;
}