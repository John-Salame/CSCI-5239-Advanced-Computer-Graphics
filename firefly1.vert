// John Salame
// Light objects using fireflies as point lights with attenuation and range
// Lighting from fireflies does not account for normals
// The deceptively named shader is actually used for grass.
#version 400 core

uniform mat4 ViewMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelViewProjectionMatrix;
uniform mat3 NormalMatrix;
vec4 adjustedVertex;

// Light propeties (taken from example 6)
uniform float fov;
uniform vec4 Global;
uniform vec4 Ambient;
uniform vec4 Diffuse;
uniform vec4 Specular;
uniform vec4 Position;

// where the modelViewMatrix of the lights is and the relative positions to that matrix
uniform mat4 fireflyModelView;
uniform vec4 firefly1;
uniform vec4 firefly2;
uniform vec4 firefly3;
uniform vec4 firefly4;

// change the height of anything which has non-zero y value
uniform sampler3D grassHeights;

//  Vertex attributes (input)
in vec4 Vertex;
in vec3 Normal;
in vec4 Color;
in vec2 Texture;

out vec4 FrontColor;
out vec2 Texcoord;


// vertex position and firefly position
vec4 applyFirefly(vec4 pos, vec4 firefly) {
  vec4 retColor = vec4(0.0);
  vec4 fireflyPos = fireflyModelView * firefly;
  vec3 diff = pos.xyz - fireflyPos.xyz;
  float mag = dot(diff, diff);
  retColor = vec4(0.1, 0.05, 0.0, 1.0) / (mag+0.1);
  return retColor;
}

// Taken from example 6
vec4 phong()
{
   //  P is the vertex coordinate on body
   vec3 P = vec3(ModelViewMatrix * adjustedVertex);
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
  float grassHeight = 0.8 + 3 * texture(grassHeights, pos.xyz).y;  // range of heights is [0.8, 1.2]
  adjustedVertex = vec4(Vertex.x, Vertex.y * grassHeight, Vertex.z, 1.0);
  // recalculate position using new modelview vertex
  pos = ModelViewMatrix * adjustedVertex;
  vec4 color = phong();
  // color grass more yellow if close to a firefly
  color += applyFirefly(pos, firefly1);
  color += applyFirefly(pos, firefly2);
  color += applyFirefly(pos, firefly3);
  color += applyFirefly(pos, firefly4);
  gl_Position = ModelViewProjectionMatrix * adjustedVertex;
  FrontColor = color;
  Texcoord = Texture;
}