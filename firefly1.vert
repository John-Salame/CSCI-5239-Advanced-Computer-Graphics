// John Salame
// Light objects using fireflies as point lights with attenuation and range
// Lighting from fireflies does not account for normals
// The deceptively named shader is actually used for grass.
#version 400 core

uniform mat4 ViewMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;

// Light propeties (taken from example 6)
uniform float fov;
uniform vec4 Global;
uniform vec4 Ambient;
uniform vec4 Diffuse;
uniform vec4 Specular;
uniform vec4 Position;

// where the modelViewMatrix of the lights is and the relative positions to that matrix
uniform mat4 fireflyModelView;
uniform vec4[100] fireflies; // array of firefly positions (the size here is the maximum I will allow)
uniform int numFireflies;
uniform float t;

// change the height of anything which has non-zero y value
uniform sampler3D noiseTexture;

//  Vertex attributes (input)
in vec4 Vertex;
in vec3 Normal;
in vec4 Color;
in vec2 Texture;

out vec4 FrontColor;
out vec2 Texcoord;


// vertex position and firefly position
vec4 applyFireflyLight(vec4 pos, vec4 firefly, float intensity) {
  vec4 retColor = vec4(0.0);
  // add some noise as a function of time to firefly before calculating the position in model view coordinates.
  // this creates flicker in the light.
  vec4 fireflyPos = fireflyModelView * (firefly + texture(noiseTexture, firefly.xyz + vec3(t)) - vec4(0.25, 0.125, 0.0625, 0.03125));
  vec3 diff = pos.xyz - fireflyPos.xyz;
  float mag = dot(diff, diff);
  retColor = intensity * vec4(0.1, 0.05, 0.0, 1.0) / (mag+0.1);
  return retColor;
}

// Adapted from example 6
// pos is the vertex position multiplied by ModelViewMatrix
vec4 phong(vec4 pos)
{
   //  P is the vertex coordinate on body
   vec3 P = vec3(pos);
   //  N is the object normal at P
   vec3 N = normalize(NormalMatrix * Normal);
   //  L is the light vector
   vec3 L = normalize(vec3(ViewMatrix*Position) - P);

   //  Emission and ambient color
   vec4 color = (Global+Ambient)*Color;

   //  Diffuse light intensity is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse - material color from Color
      color += Id*Diffuse*Color;
      //  R is the reflected light vector R = 2(L.N)N - L
      vec3 R = reflect(-L, N);
      //  V is the view vector (eye at the origin)
      vec3 V = (fov>0.0) ? normalize(-P) : vec3(0,0,1);
      //  Specular is cosine of reflected and view vectors
      //  Assert material specular color is white
      float Is = dot(R,V);
      if (Is>0.0) color += pow(Is,8.0)*Specular;
   }

   //  Return sum of color components
   return color;
}

void main() {
  // calculate where the vertex would normally be
  vec4 pos = ModelViewMatrix * Vertex;
  vec4 color = phong(pos);
  // color grass more yellow if close to a firefly
  for(int i = 0; i < numFireflies; i++) {
    color += applyFireflyLight(pos, fireflies[i], 10.0 / numFireflies);
  }
  gl_Position = ModelViewProjectionMatrix * Vertex;
  FrontColor = color;
  Texcoord = Texture;
}