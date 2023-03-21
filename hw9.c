/*
 * John Salame
 * CSCI 5239 Advanced Computer Graphics
 * Homework 9
 *
 *  Key bindings:
 *  m          Toggle shader
 *  o          Change objects
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include <stdlib.h>
#include "CSCIx239.h"
#include "grass.h"
int mode=0;    //  Shader
int th=0,ph=0; //  View angles
int fov=57;    //  Field of view (for perspective)
int tex=0;     //  Texture
int obj=1;     //  Object
#define NUM_SHADERS 2
// int shader[NUM_SHADERS];  //  Shader
float asp=1;   //  Aspect ratio
float dim=3;   //  Size of world
float lTh = 0.0; // theta for calculating light position
// const char* text[NUM_SHADERS] = {"Blur Filter","Prewitt Filter"};
int width = 0;
int height = 0;

// grass stuff
int numPatches = 3;
int oldNumPatches = 0;
double* grassHeights = 0; // will be a 2d array of grass heights
float* grassDataMain = 0;
unsigned int grassVbo = 0;
unsigned int grassVao = 0;
// base plate with grass texture
unsigned int baseVbo = 0;
unsigned int baseVao = 0;
unsigned int grassTexture = 0;
unsigned int starTexture = 0; // might be good for anything that twinkles
unsigned int blankTexture = 0; // a single white pixel, used for untextured objects

// OPENGL4 STUFF
unsigned int id = 0; // shader input locations
float viewMat[16];
float modelViewMat[16];
float projectionMat[16];
float modelView1[16]; // temporary solution to glPushMatrix()
float modelView2[16]; // temporary solution to glPushMatrix()
// shader program names
unsigned int simpleShader = 0;
unsigned int fireflyShader = 0;
unsigned int textureShader = 0;
unsigned int particleShader = 0;
unsigned int computeShader = 0;
unsigned int canvasVbo = 0;
unsigned int canvasVao = 0;

// HW 7
unsigned int noiseTexture = 0;

// HW 9 - compute shader
int nw, ng, maxParticles; //  Work group size and count
unsigned int posbuf = 0; //  Position buffer
unsigned int velbuf = 0; //  Velocity buffer
unsigned int lifetimebuf = 0; // lifetime of each individual particle
unsigned int colbuf; //  Color buffer
float minLifetime = 0.5; // how long it takes for the particle to appear
float lifespan = 3.0; // lifespan in seconds (maximum lifetime)
int numSparks = 0;
int numFireflies = 4;
double oldTime;
double currentTime;
float dt; // delta time, used for aging the lifetime
float fireflyColors[16]; // set this in main()


typedef struct
{
  union { float x; float r; };
  union { float y; float g; };
  union { float z; float b; };
  union { float w; float a; };
} vec4;

// GEOMETRY
const float canvas[] = {
//  X    Y   Z     tX   tY
  -1.0, 1.0,0.0,   0.0,1.0,
  -1.0,-1.0,0.0,   0.0,0.0,
   1.0, 1.0,0.0,   1.0,1.0,
   1.0, 1.0,0.0,   1.0,1.0,
  -1.0,-1.0,0.0,   0.0,0.0,
   1.0,-1.0,0.0,   1.0,0.0
};

const float basePlate[] = {
//  X    Y     Z      nx  ny  nz     R   G   B      tX  tY
  -5.0, 0.0, -5.0,   0.0,1.0,0.0,   1.0,1.0,1.0,   0.0,1.0,
  -5.0, 0.0,  5.0,   0.0,1.0,0.0,   1.0,1.0,1.0,   0.0,0.0,
  5.0,  0.0, -5.0,   0.0,1.0,0.0,   1.0,1.0,1.0,   1.0,1.0,
  5.0,  0.0, -5.0,   0.0,1.0,0.0,   1.0,1.0,1.0,   1.0,1.0,
  -5.0, 0.0,  5.0,   0.0,1.0,0.0,   1.0,1.0,1.0,   0.0,0.0,
  5.0,  0.0,  5.0,   0.0,1.0,0.0,   1.0,1.0,1.0,   1.0,0.0,
};

// Helper function for updating matrices in shader
void PassMatricesToShader(int shaderProgram, float viewMat[], float modelViewMat[], float projectionMat[]) {
  float modelViewProjectionMat[16];
  float normalMat[9];
  mat4identity(modelViewProjectionMat);
  // modelViewProjectionMat = projectionMat * modelViewMat
  mat4multMatrix(modelViewProjectionMat, projectionMat);
  mat4multMatrix(modelViewProjectionMat, modelViewMat);
  // calculate normal matrix from ModelViewMatrix
  mat4normalMatrix(modelViewMat, normalMat);
  int id = glGetUniformLocation(shaderProgram, "ViewMatrix");
  glUniformMatrix4fv(id, 1, 0, viewMat);
  id = glGetUniformLocation(shaderProgram, "ModelViewMatrix");
  glUniformMatrix4fv(id, 1, 0, modelViewMat);
  id = glGetUniformLocation(shaderProgram, "ModelViewProjectionMatrix");
  glUniformMatrix4fv(id, 1, 0, modelViewProjectionMat);
  id = glGetUniformLocation(shaderProgram, "NormalMatrix");
  glUniformMatrix3fv(id, 1, 0, normalMat);
  ErrCheck("PassMatricesToShader()");
}

void DrawCanvas(unsigned int shader, unsigned int texture) {
  //  Initialize VBO on first use
  if (!canvasVbo)
  {
    //  Get buffer name
    glGenBuffers(1, &canvasVbo);
    //  Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, canvasVbo);
    //  Copy icosahedron to VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(canvas), canvas, GL_STATIC_DRAW);
  }
  //  On subsequanet calls, just bind VBO
  else
    glBindBuffer(GL_ARRAY_BUFFER, canvasVbo);
  // Initialize the VAO on first use
  if (!canvasVao) {
    glGenVertexArrays(1, &canvasVao);
    glBindVertexArray(canvasVao); // use the VAO
    glBindBuffer(GL_ARRAY_BUFFER, canvasVbo); // Bind VBO
    int loc = glGetAttribLocation(shader, "Vertex");
    glVertexAttribPointer(loc, 3, GL_FLOAT, 0, 20, (void*)0);
    glEnableVertexAttribArray(loc);
    loc = glGetAttribLocation(shader, "Texture");
    glVertexAttribPointer(loc, 2, GL_FLOAT, 0, 20, (void*)12);
    glEnableVertexAttribArray(loc);
  }

  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(canvasVao); // use the VAO
  //  Draw canvas rectangle
  glDrawArrays(GL_TRIANGLES, 0, 6);
  //  Release VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //  Release VAO
  glBindVertexArray(0);
  // Reset texture
  glBindTexture(GL_TEXTURE_2D, blankTexture);
  ErrCheck("draw canvas");
}

void DrawBasePlate(unsigned int shader, unsigned int texture) {
    glUseProgram(shader);
  if (baseVbo == 0) {
    glGenBuffers(1, &baseVbo);
    glBindBuffer(GL_ARRAY_BUFFER, baseVbo); // use VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(basePlate), basePlate, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Release VBO
  }
  ErrCheck("base plate vbo");
  // Apply VAO
  if (baseVao == 0) {
    glGenVertexArrays(1, &baseVao);
    glBindVertexArray(baseVao); // use VAO
    glBindBuffer(GL_ARRAY_BUFFER, baseVbo); // use VBO
    ErrCheck("base plate vao beginning");
    // Bind attribute locations
    int loc = glGetAttribLocation(shader, "Vertex");
    glVertexAttribPointer(loc, 3, GL_FLOAT, 0, 44, (void*)0);
    glEnableVertexAttribArray(loc);
    ErrCheck("base plate Vertex");
    //  Normal
    loc = glGetAttribLocation(shader, "Normal");
    glVertexAttribPointer(loc, 3, GL_FLOAT, 0, 44, (void*)12);
    glEnableVertexAttribArray(loc);
    ErrCheck("base plate Normal");
    //  Color
    loc = glGetAttribLocation(shader, "Color");
    glVertexAttribPointer(loc, 3, GL_FLOAT, 0, 44, (void*)24);
    glEnableVertexAttribArray(loc);
    ErrCheck("base plate Color");
    //  Texture
    loc = glGetAttribLocation(shader, "Texture");
    glVertexAttribPointer(loc, 2, GL_FLOAT, 0, 44, (void*)36);
    glEnableVertexAttribArray(loc);
    ErrCheck("base plate Texture");
    // Release VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Release VAO
    glBindVertexArray(0);
  }
  ErrCheck("base plate vao");
  // Draw the base plate
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(baseVao); // use VAO
  glDrawArrays(GL_TRIANGLES, 0, 6);
  // Release VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // Release VAO
  glBindVertexArray(0);
  // Reset texture
  glBindTexture(GL_TEXTURE_2D, blankTexture);
  ErrCheck("base plate");
}

//
//  Random numbers from min to max
//  Taken from Example 19
//
static float frand(float min, float max)
{
    return rand() * (max - min) / RAND_MAX + min;
}

// create new random values and place them in shader storage buffers using memory mapping
// Adapted from Example 19
int ResetSparks(int numSparks, float fireflyColors[]) {
  vec4* pos, * vel, *col;
  float* lifetime;

  if (numSparks > maxParticles) {
    numSparks = maxParticles;
  }

  // random initial positions very close to the firefly
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, posbuf);
  pos = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++)
  {
    pos[i].x = frand(-0.05, 0.05);
    pos[i].y = frand(-0.05, 0.05);
    pos[i].z = frand(-0.05, 0.05);
    pos[i].w = 1;
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  // random velocities
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, velbuf);
  vel = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++)
  {
    vel[i].x = frand(-0.3, 0.3);
    vel[i].y = frand(-0.2, 0.4);
    vel[i].z = frand(-0.3, 0.3);
    vel[i].w = 1;
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  // slightly randomized colors
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, colbuf);
  col = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++)
  {
    col[i].r = frand(0.9, 1.1) * fireflyColors[0];
    col[i].g = frand(0.9, 1.1) * fireflyColors[1];
    col[i].b = frand(0.9, 1.1) * fireflyColors[2];
    col[i].a = fireflyColors[3];
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  // every particle starts with a lifetime between minLifetime and lifespan (max lifetime)
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifetimebuf);
  lifetime = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++) {
    lifetime[i] = frand(0.0, lifespan);
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  return numSparks;
}

// prepare the compute shader and storage buffers
// Adapted from Example 19
int InitSparks(int numSparks, float fireflyColors[]) {
  //  Get max workgroup size and count
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &ng);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &nw);
  if (ng > 8192) ng = 8192;
  maxParticles = nw * ng;

  if (numSparks > maxParticles) {
      numSparks = maxParticles;
  }

  //  Initialize position buffer
  glGenBuffers(1, &posbuf);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, posbuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numSparks * sizeof(vec4), NULL, GL_STATIC_DRAW);

  //  Initialize velocity buffer
  glGenBuffers(1, &velbuf);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, velbuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numSparks * sizeof(vec4), NULL, GL_STATIC_DRAW);

  //  Initialize color buffer
  glGenBuffers(1, &colbuf);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, colbuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numSparks * sizeof(vec4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  //  Initialize lifetime buffer
  glGenBuffers(1, &lifetimebuf);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifetimebuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numSparks * sizeof(float), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  //  Set buffer base
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posbuf);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velbuf);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lifetimebuf);

  // provide random values to the buffers and return the number of particles the compute shader will handle
  return ResetSparks(numSparks, fireflyColors);
}


// draw sparks around fireflies
void DrawFireflies(float fireflyPositions[], int numSparks, int numFireflies) {
  unsigned int currentShader = simpleShader;
  // first, draw the fireflies
  for (int i = 0; i < 4; ++i) {
    mat4copy(modelView1, modelViewMat); // replacement for glPushMatrix()
    mat4translate(modelViewMat, fireflyPositions[4 * i + 0], fireflyPositions[4 * i + 1], fireflyPositions[4 * i + 2]);
    mat4scale(modelViewMat, 0.05, 0.05, 0.05);
    PassMatricesToShader(currentShader, viewMat, modelViewMat, projectionMat);
    SimpleIcosahedron(currentShader); // represents a point light
    mat4copy(modelViewMat, modelView1); // replacement for glPopMatrix()
  }
  ErrCheck("fireflies draw");  
      
      
  currentShader = computeShader;
  //  Launch compute shader
  glUseProgram(currentShader);
  unsigned int id = glGetUniformLocation(currentShader, "lifespan");
  glUniform1f(id, lifespan);
  id = glGetUniformLocation(currentShader, "deltaTime");
  glUniform1f(id, dt);
  // glDispatchComputeGroupSizeARB(n/nw,1,1,nw,1,1);
  glDispatchCompute(maxParticles / nw, 1, 1); // compensate for inability to use extension on my computer

  // PREPARE TO DRAW FIREFLY SPARKS using particle shader -- transparent stuff is drawn last
  // Example 16 is used as reference, and some code is borrowed.
  currentShader = particleShader;
  glUseProgram(currentShader); // this should be a particle shader
  //  Set particle size
  glPointSize(10); // use small particles
  // glPointSize(25);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, starTexture);
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glDepthMask(0); // disable z-buffer

  // prepare vertex attributes
  id = glGetUniformLocation(currentShader, "tex");
  glUniform1i(id, 0); // texture unit 0

  // use positions calculated by compute shader
  glBindBuffer(GL_ARRAY_BUFFER, posbuf);
  id = glGetAttribLocation(currentShader, "Offset");
  glVertexAttribPointer(id, 4, GL_FLOAT, 0, 16, (void*)0);
  glEnableVertexAttribArray(id);

  // use colors from the shader storage buffer
  glBindBuffer(GL_ARRAY_BUFFER, colbuf);
  id = glGetAttribLocation(currentShader, "Color");
  glVertexAttribPointer(id, 4, GL_FLOAT, 0, 16, (void*)0);
  glEnableVertexAttribArray(id);
  ErrCheck("set up to draw firefly particles");

  // draw the fireflies
  PassMatricesToShader(currentShader, viewMat, modelViewMat, projectionMat);
  int numSparksPerFirefly = numSparks / numFireflies;
  for (int i = 0; i < numFireflies; i++) {
    // set the location of the firefly who is the source of this spark
    id = glGetAttribLocation(currentShader, "ParentVertex");
    glUniform4fv(id, 1, fireflyPositions + 4 * i); // 4 is the number of elements in the position
    glDrawArrays(GL_POINTS, i * numSparksPerFirefly, numSparksPerFirefly);
    ErrCheck("draw firefly sparks loop");
  }

  // undo firefly settings
  glDisable(GL_POINT_SPRITE);
  glDisable(GL_BLEND);
  glDepthMask(1);
  id = glGetAttribLocation(currentShader, "Offset");
  glDisableVertexAttribArray(id);
  id = glGetAttribLocation(currentShader, "Color");
  glDisableVertexAttribArray(id);
  glBindTexture(GL_TEXTURE_2D, blankTexture);
  ErrCheck("firefly sparks draw");
}


void Exit() {
  glDeleteBuffers(1, &grassVbo);
  glDeleteBuffers(1, &baseVbo);
  CleanupIcosahedron();
}
// default view when you start the program or press 0
void ResetView() {
    ph = 30;
    th = 0;
    ResetSparks(numSparks, fireflyColors);
}

//
//  Refresh display
//
void display(GLFWwindow* window)
{
  //  Erase the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //  Enable Z-buffering in OpenGL
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  /*
  //  Set material and lighting interaction (not sure if this matters outside of fixed pipeline)
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL); // without this enabled, glColor4fv does not apply, but the materials do
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // mix colors
  */

  //  Set view
  ViewGL4(viewMat, th, ph, fov, dim);
  mat4copy(modelViewMat, viewMat);

  // Place the light using shader 0 (simple shader)
  glUseProgram(simpleShader); // simple shader
  float position[] = { 3 * Cos(lTh), 1.0, 3 * Sin(lTh), 1.0 };
  mat4copy(modelView1, modelViewMat); // replacement for glPushMatrix()
  // translate and scale down the icosahedron
  mat4translate(modelViewMat, position[0], position[1], position[2]);
  mat4scale(modelViewMat, 0.2, 0.2, 0.2);
  PassMatricesToShader(simpleShader, viewMat, modelViewMat, projectionMat);
  SimpleIcosahedron(simpleShader); // represents a point light
  mat4copy(modelViewMat, modelView1); // replacement for glPopMatrix()
  ErrCheck("Icosahedron light");

  // Set the light parameters
  // Note: I don't have any material properties, so color is used as the material property for every type of lighting.
  glUseProgram(fireflyShader);
  float global[] = { 0.1,0.1,0.1,1.0 };
  float ambient[] = { 0.2,0.2,0.2,1.0 };
  float diffuse[] = { 0.5,0.5,0.5,1.0 };
  float specular[] = { 0.2,0.2,0.2,1.0 };
  //  Set light property uniforms
  id = glGetUniformLocation(fireflyShader, "fov");
  glUniform1f(id, fov);
  id = glGetUniformLocation(fireflyShader, "Global");
  glUniform4fv(id, 1, global);
  id = glGetUniformLocation(fireflyShader, "Ambient");
  glUniform4fv(id, 1, ambient);
  id = glGetUniformLocation(fireflyShader, "Diffuse");
  glUniform4fv(id, 1, diffuse);
  id = glGetUniformLocation(fireflyShader, "Specular");
  glUniform4fv(id, 1, specular);
  id = glGetUniformLocation(fireflyShader, "Position");
  glUniform4fv(id, 1, position);
  // Set texture units for samplers
  id = glGetUniformLocation(fireflyShader, "tex");
  glUniform1i(id, 0); // texture unit 0
  id = glGetUniformLocation(fireflyShader, "grassHeights");
  glUniform1i(id, 1); // texture unit 1 (noise)

  // draw the base plate
  PassMatricesToShader(fireflyShader, viewMat, modelViewMat, projectionMat);
  ErrCheck("before base plate");
  DrawBasePlate(fireflyShader, grassTexture);

  
  // set up the fireflies
  float fireflyModelViewMat[16];
  // hold the positions of four fireflies
  float fireflyPositions[16] = 
  { 1.0, 1.0, 2.0, 1.0 ,
   -1.0, 1.0, 1.0, 1.0 ,
   0.0, 1.0, -2.0, 1.0 ,
   0.5, 0.5, 0.0, 1.0 };
  mat4copy(fireflyModelViewMat, modelViewMat); // store the current modelViewMatrix so we can place the fireflies in the scene and remember their positions for later

  // make the fireflies move a bit in a cyclical way
  for (int i = 0; i < 4; ++i) {
      float x = fireflyPositions[4 * i + 0];
      float y = fireflyPositions[4 * i + 1];
      float phase = x - y; // make the fireflies a bit out of phase with each other in their motion
      phase = x - phase * phase;
      phase = fmin(2.0, abs(phase)); // don't have hyperactive fireflies
      fireflyPositions[4 * i + 0] += 0.3 * Cos(phase * lTh) +-0.1 * fmod(x, 2);
      fireflyPositions[4 * i + 1] += 0.05 * Cos(10*lTh) * Sin(20*lTh) + 0.2 * Cos(phase*lTh);
      fireflyPositions[4 * i + 2] += 0.2*(0.5-Sin(phase*lTh));
  }

  // use the firefly shader and draw grass lit by fireflies
  glUseProgram(fireflyShader);
  mat4copy(fireflyModelViewMat, modelViewMat); // store the current modelViewMatrix so we can place the fireflies in the scene and remember their positions for later
  
  // pass the position of the fireflies for lighting purposes
  id = glGetUniformLocation(fireflyShader, "fireflyModelView");
  glUniformMatrix4fv(id, 1, 0, fireflyModelViewMat);
  id = glGetUniformLocation(fireflyShader, "fireflies");
  glUniform4fv(id, 4, fireflyPositions);
  // lTh is a convenient variable that changes with time. I can use it to oscillate through a portion of the 3D noise map in order to make the fireflies quake.
  id = glGetUniformLocation(fireflyShader, "t");
  glUniform1f(id, (float) lTh);
  ErrCheck("fireflies lighting");
  

  // make a lawn of 6 by 10 blades of grass per patch. Without scaling, a patch takes up a [-1, 1] area.
  // place the blades 0.3 apart horizontally and 0.2 apart along z
  // if the number of grass patches changed, then destroy the existing grassHeights array and create a new one
  if (numPatches != oldNumPatches) {
    oldNumPatches = numPatches;
    if (grassHeights) {
      free(grassHeights);
    }
    if (grassDataMain) {
        free(grassDataMain);
    }
    // create the buffers
    int numBlades = 10*numPatches*6*numPatches;
    printf("Number of blades: %d\n", numBlades);
    grassHeights = malloc(numBlades*sizeof(double));
    // initialze the random heights
    for (int i = 0; i < numBlades; i++) {
      grassHeights[i] = 0.1 * ((rand() % 5) + 8); // range of heights is [0.8, 1.2]
    }
    // create the grass vao and vbo
    int grassVboSize = getSizeOfGrassVBO(numBlades);
    printf("Allocating grass VBO of size %d\n", grassVboSize);
    grassDataMain = malloc(grassVboSize);
    copyGrassData(grassDataMain, numBlades);
    /*
    // adjust grass heights from main program
    int grassVertices = getNumVerticesPerGrass();
    float* seek = grassDataMain + 1; // start at the first y value
    for (int i = 0; i < numBlades; i++) {
      for (int j = 0; j < grassVertices; j++) {
        *(seek) *= grassHeights[i];
        seek += 13; // skip 13 floats since that is how many attributes exist per vertex
      }
    }
    */
    InitGrassVBO(grassDataMain, grassVboSize, &grassVbo);
    InitGrassVAO(fireflyShader, &grassVbo, &grassVao);
    ErrCheck("grass main");
  }


  // Now draw the lawn

  glUseProgram(fireflyShader);
  //  Bind attribute arrays using VAO (VAO knows VBO to use)
  glBindVertexArray(grassVao);
  int grassVertices = getNumVerticesPerGrass(); // number of vertices per blade of grass
  int drawStart = 0; // offset to draw from in glDrawArrays()

  mat4copy(modelView1, modelViewMat); // replacement for glPushMatrix()
  mat4translate(modelViewMat, 0.0, 0.0, -numPatches); // start at the back row
  for(int i = 0; i < 10*numPatches; i++) {
    mat4copy(modelView2, modelViewMat); // push matrix
    mat4translate(modelViewMat, -numPatches, 0.0, 0.0); // start of row (left side)
    // travserse the row
    for (int j = 0; j < 6*numPatches; j++) {
      PassMatricesToShader(fireflyShader, viewMat, modelViewMat, projectionMat);
      glDrawArrays(GL_TRIANGLES, drawStart, grassVertices);
      mat4translate(modelViewMat, 0.30, 0.0, 0.0);
      drawStart += grassVertices;
    }
    mat4copy(modelViewMat, modelView2); // replacement for glPopMatrix()
    mat4translate(modelViewMat, 0.0, 0.0, 0.2); // change the row
  }
  mat4copy(modelViewMat, modelView1); // replacement for glPopMatrix()
  PassMatricesToShader(fireflyShader, viewMat, modelViewMat, projectionMat);

  //  Release attribute arrays
  glBindVertexArray(0);

  // Draw firefly sparks
  DrawFireflies(fireflyPositions, numSparks, numFireflies);

  //  Revert to fixed pipeline
  glUseProgram(0);


#ifndef APPLE_GL4
  //  Display axes
  // Axes(2);
#endif
  
  
#ifndef APPLE_GL4
  //  Display parameters
  SetColor(1, 1, 1);
  glWindowPos2i(5, 5);
  int fps = FramesPerSecond();
  // Print("Angle=%d,%d  Dim=%.1f Projection=%s Mode=%s Frames Per Second=%d", th, ph, dim, fov > 0 ? "Perpective" : "Orthogonal", text[mode], fps);
  Print("Angle=%d,%d  Dim=%.1f Projection=%s Frames Per Second=%d", th, ph, dim, fov > 0 ? "Perpective" : "Orthogonal", fps);
#endif

  //  Render the scene and make it visible
  ErrCheck("display");
  glFlush();
  glfwSwapBuffers(window);
}

//
//  Key pressed callback
//
void key(GLFWwindow* window,int key,int scancode,int action,int mods)
{
   //  Discard key releases (keeps PRESS and REPEAT)
   if (action==GLFW_RELEASE) return;

   //  Exit on ESC
   if (key == GLFW_KEY_ESCAPE)
       glfwSetWindowShouldClose(window, 1);
   //  Reset view angle
   else if (key == GLFW_KEY_0) {
       ResetView();
   }
   //  Switch shaders
   else if (key == GLFW_KEY_M)
       mode = (mode + 1) % NUM_SHADERS;
   //  Switch objects
   else if (key == GLFW_KEY_O)
       obj = 1 - obj;
   //  Switch between perspective/orthogonal
   else if (key == GLFW_KEY_P)
       fov = fov ? 0 : 57;
   //  Increase/decrease asimuth
   else if (key==GLFW_KEY_RIGHT)
      th += 5;
   else if (key==GLFW_KEY_LEFT)
      th -= 5;
   //  Increase/decrease elevation
   else if (key==GLFW_KEY_UP)
      ph += 5;
   else if (key==GLFW_KEY_DOWN)
      ph -= 5;
   //  PageDown key - increase dim (zoom out)
   else if (key==GLFW_KEY_PAGE_DOWN || key == GLFW_KEY_LEFT_BRACKET)
      dim += 0.1;
   //  PageUp key - decrease dim (zoom in)
   else if ((key==GLFW_KEY_PAGE_UP || key == GLFW_KEY_RIGHT_BRACKET) && dim > 1)
      dim -= 0.1;
   // Plus sign - increase number of grass patches
   else if (mods==GLFW_MOD_SHIFT && key==GLFW_KEY_EQUAL) {
       numPatches++;
   }
   else if (key == GLFW_KEY_MINUS) {
       numPatches = (numPatches > 0 ? numPatches-1 : 1);
   }

   //  Wrap angles
   th %= 360;
   ph %= 360;
   //  Update projection
   ProjectionGL4(modelViewMat, projectionMat, fov,asp,dim);
}

//
//  Window resized callback
//
void reshape(GLFWwindow* window,int width,int height)
{
   //  Get framebuffer dimensions (makes Apple work right)
   glfwGetFramebufferSize(window,&width,&height);
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   ProjectionGL4(modelViewMat, projectionMat, fov,asp,dim);
   ErrCheck("Reshape");
}


//
//  Create Shader Program with Location Names
//
int CreateShaderProgCompute(char* file)
{
    //  Create program
    int prog = glCreateProgram();
    //  Create and compile compute shader
    CreateShader(prog, GL_COMPUTE_SHADER, file);
    //  Link program
    glLinkProgram(prog);
    //  Check for errors
    PrintProgramLog(prog);
    //  Return name
    return prog;
}

//
//  Main program with GLFW event loop
//
int main(int argc,char* argv[])
{
  //  Initialize GLFW
  GLFWwindow* window = InitWindow("John Salame HW 9 - Advanced Shaders (Compute Shader)",0,600,600,&reshape,&key);

  //  Load shader
  simpleShader = CreateShaderProg("simple.vert", "simple.frag");
  fireflyShader = CreateShaderProg("firefly1.vert", "texture.frag");
  textureShader = CreateShaderProg("texture.vert", "texture.frag");
  particleShader = CreateShaderProg("particle.vert", "particle.frag");
  computeShader = CreateShaderProgCompute("spark.cs");
  // shader[0] = CreateShaderProg("texture.vert", "blur.frag"); // filter
  // shader[1] = CreateShaderProg("texture.vert", "prewitt.frag"); // filter
  //  Load textures
  glActiveTexture(GL_TEXTURE0);
  tex = LoadTexBMP("pi.bmp");
  grassTexture = LoadTexBMP("grass.bmp");
  starTexture = LoadTexBMP("star.bmp");
  blankTexture = LoadTexBMP("blank.bmp");
  noiseTexture = CreateNoise3D(GL_TEXTURE1); // put noise in texture unit 1


  // make the fireflies slightly yellow and transparent
  for (int i = 0; i < 4; ++i) {
      fireflyColors[4 * i + 0] = 0.8;
      fireflyColors[4 * i + 1] = 1.0; // greenish color
      fireflyColors[4 * i + 2] = 0.4;
      fireflyColors[4 * i + 3] = 0.6; // slightly transparent
  }
  // Initialize things related to firefly sparks (for example, the compute shader and shader storage buffers)
  int sparksPerFirefly = 10;
  numSparks = InitSparks(numFireflies * sparksPerFirefly, fireflyColors);

  //  Event loop
  ErrCheck("init");
  ResetView();
  while(!glfwWindowShouldClose(window))
  {
    oldTime = currentTime;
    currentTime = glfwGetTime();
    // Move the light
    lTh = fmod(90 * currentTime, 360);
    dt = (float) currentTime - oldTime;
    //  Display
    display(window);
    //  Process any events
    glfwPollEvents();
  }
  //  Shut down GLFW
  glfwDestroyWindow(window);
  glfwTerminate();
  Exit();
  return 0;
}
