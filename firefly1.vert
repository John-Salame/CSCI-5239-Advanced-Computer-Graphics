// John Salame
// Light objects using fireflies as point lights with attenuation and range
// Lighting does not account for normals
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

//  Vertex attributes (input)
in vec4 Vertex;
in vec3 Normal;
in vec4 Color;
in vec2 Texture;

out vec4 FrontColor;

const float squaredRange = 0.25;

// Taken from example 6
vec4 phong()
{
   //  P is the vertex coordinate on body
   vec3 P = vec3(ModelViewMatrix * Vertex);
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
  vec4 pos = ModelViewMatrix * Vertex;
  vec4 color = phong();
  // color grass more yellow if close to a firefly
  for(int i = 0; i < 4; i++) {
    vec4 fireflyPos = gl_LightSource[i].position;
    vec3 diff = pos.xyz - fireflyPos.xyz;
    // color = vec4(diff, 1.0);
    float mag = dot(diff, diff);
    if(mag < squaredRange) {
      color += vec4(0.1, 0.05, 0.0, 1.0) / (mag+0.1);
    }
  }
  gl_Position = ModelViewProjectionMatrix * Vertex;
  FrontColor = color;
}