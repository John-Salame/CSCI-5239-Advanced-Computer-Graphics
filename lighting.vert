// John Salame
// Per-vertex lighting with an intensity interpolated to the frag shader
// Currently it is set up to work with only one light source, which is a single point light.
#version 120

// Intensity is a combination of ambient, diffuse, and specular. The RGB values may vary.
varying vec4 intensity;

void main() {
  vec4 P = gl_ModelViewMatrix * gl_Vertex;
  vec3 N = gl_NormalMatrix * gl_Normal;
  vec3 L = normalize(gl_LightSource[0].position.xyz - P.xyz);
  vec3 V = normalize(-P.xyz); // view vector (for Specular)
  vec3 R = reflect(-L, N);

  // Diffuse intensity is non-negative dot product of light and normal vectors.
  float Id = max(dot(L,N), 0.0);
  // Specular intensity is non-negative dot product of view and reflection vector if the surface is lit
  float Is = (Id > 0.0) ? pow(max(dot(R,V), 0.0), gl_FrontMaterial.shininess) : 0.0;

  intensity = (gl_FrontLightProduct[0].ambient
    + Id*gl_FrontLightProduct[0].diffuse
    + Is*gl_FrontLightProduct[0].specular);
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_FrontColor = gl_Color * intensity; // do this in case frag shader does not exist
  gl_TexCoord[0] = gl_MultiTexCoord0; // pass texture coordinates to fragment shader
}