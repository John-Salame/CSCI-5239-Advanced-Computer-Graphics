/*
 * John Salame
 * CSCI 5239 Advanced Computer Graphics
 * Homework 10 - Compute Shader
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
int mode=1;    //  Enable boids compute shader
int th=0,ph=0; //  View angles
int fov=57;    //  Field of view (for perspective)
int tex=0;     //  Texture
int obj=1;     //  Object
#define NUM_SHADERS 2
// int shader[NUM_SHADERS];  //  Shader
float asp=1;   //  Aspect ratio
float dim=3;   //  Size of world
float lTh = 0.0; // theta for calculating light position
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
float fireflyModelViewMat[16];
float modelView1[16]; // temporary solution to glPushMatrix()
float modelView2[16]; // temporary solution to glPushMatrix()
// shader program names
unsigned int simpleShader = 0;
unsigned int fireflyShader = 0;
unsigned int textureShader = 0;
unsigned int particleShader = 0;
unsigned int computeShader = 0;
unsigned int boidsComputeShader = 0;
unsigned int canvasVbo = 0;
unsigned int canvasVao = 0;

// HW 7
unsigned int noiseTexture = 0;

// HW 9 - compute shader
int nw, ng, maxParticles; //  Work group size and count
unsigned int fireflyPosBuf = 0;
unsigned int fireflyVelBuf = 0;
unsigned int posbuf = 0; //  Position buffer
unsigned int velbuf = 0; //  Velocity buffer
unsigned int lifetimebuf = 0; // lifetime of each individual particle
unsigned int colbuf; //  Color buffer
float lifespan = 3.0; // lifespan in seconds (maximum lifetime) of spark particle
int numSparks = 0; // calculate in main method
const int sparksPerFirefly = 5;
const int sparksPerWorkGroup = 100; // must be a multiple of sparksPerFirefly.
const int firefliesPerSwarm = 10;
const int numFireflySwarms = 6; // initial number of swarms (they may merge later on)
int numFireflies = 0; // calculate in InitFireflies.
const int maxFireflies = 100;
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

void ResetFireflies(int firefliesPerSwarm, int numFireflySwarms, int numFireflies) {
  // I have no clue what would happen if numFireflies changes value due to having too many swarms -> too many work groups
  vec4* pos, * vel;

  // create swarms with random initial positions
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyPosBuf);
  pos = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numFireflySwarms; ++i)
  {
    int swarmIndex = i * firefliesPerSwarm;
    vec4 swarmCenter;
    swarmCenter.x = frand(-2.5, 2.5);
    swarmCenter.y = frand(1.0, 1.5);
    swarmCenter.z = frand(-2.5, 2.5);
    for (int j = 0; j < firefliesPerSwarm; ++j) {
      pos[swarmIndex + j].x = swarmCenter.x + frand(-0.3, 0.3);
      pos[swarmIndex + j].y = swarmCenter.y + frand(-0.3, 0.3);
      pos[swarmIndex + j].z = swarmCenter.z + frand(-0.3, 0.3);
      pos[swarmIndex + j].w = 1.0;
    }
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  // random velocities
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyVelBuf);
  vel = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numFireflies; i++)
  {
    vel[i].x = frand(-1.5, 1.5);
    vel[i].y = frand(-0.3, 0.3);
    vel[i].z = frand(-1.5, 1.5);
    vel[i].w = 1.0;
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// prepare the compute shader and storage buffers
// Adapted from Example 19
int InitFireflies(int firefliesPerSwarm, int numFireflySwarms) {
  numFireflies = firefliesPerSwarm * numFireflySwarms;
  if (numFireflies > maxParticles) {
    numFireflies = maxParticles;
  }

  //  Initialize position buffer
  glGenBuffers(1, &fireflyPosBuf);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyPosBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numFireflies * sizeof(vec4), NULL, GL_STATIC_DRAW);

  //  Initialize velocity buffer
  glGenBuffers(1, &fireflyVelBuf);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyVelBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numFireflies * sizeof(vec4), NULL, GL_STATIC_DRAW);

  // provide random values to the buffers and return the number of particles the compute shader will handle
  ResetFireflies(firefliesPerSwarm, numFireflySwarms, numFireflies);
  return numFireflies; // not really necessary, but make it more clear we're setting the value in this function.
}

void ComputeFireflyLocations() {
  // Set buffer base
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, fireflyPosBuf);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, fireflyVelBuf);

  unsigned int currentShader = boidsComputeShader;
  //  Launch compute shader
  glUseProgram(currentShader);
  id = glGetUniformLocation(currentShader, "deltaTime");
  glUniform1f(id, dt);
  id = glGetUniformLocation(currentShader, "numFireflies");
  glUniform1i(id, numFireflies);
  /*
  id = glGetUniformLocation(currentShader, "t");
  glUniform1f(id, (float)currentTime);
  */
  glDispatchCompute(ceil(1.0 * numFireflies / sparksPerWorkGroup), 1, 1); // compensate for inability to use extension on my computer

  //  Wait for compute shader
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  ErrCheck("calculate firefly positions (boids shader)");
}

// create new random values and place them in shader storage buffers using memory mapping
// Adapted from Example 19
void ResetSparks(int numSparks, float fireflyColors[]) {
  vec4* pos, * vel, *col;
  float* lifetime;

  // random initial positions very close to the firefly
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, posbuf);
  pos = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++)
  {
    pos[i].x = pos[i].y = pos[i].z = 0.0;
    pos[i].w = 1.0;
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  // random velocities
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, velbuf);
  vel = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++)
  {
    vel[i].x = vel[i].y = vel[i].z = 0.0;
    vel[i].w = 1.0;
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

  // every particle starts with a lifetime between 0.0 and lifespan (max lifetime)
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifetimebuf);
  lifetime = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numSparks * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  for (int i = 0; i < numSparks; i++) {
    lifetime[i] = frand(0.0, lifespan);
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// prepare the compute shader and storage buffers
// Adapted from Example 19
int InitSparks(int sparksPerFirefly, int numFireflies, float fireflyColors[]) {
  numSparks = sparksPerFirefly * numFireflies;
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

  // provide random values to the buffers and return the number of particles the compute shader will handle
  ResetSparks(numSparks, fireflyColors);
  return numSparks;
}

void ComputeSparkLocations() {
  // Set buffer base
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posbuf);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velbuf);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lifetimebuf);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, fireflyPosBuf);

  unsigned int currentShader = computeShader;
  //  Launch compute shader
  glUseProgram(currentShader);
  unsigned int id = glGetUniformLocation(currentShader, "lifespan");
  glUniform1f(id, lifespan);
  id = glGetUniformLocation(currentShader, "deltaTime");
  glUniform1f(id, dt);
  id = glGetUniformLocation(currentShader, "t");
  glUniform1f(id, (float)currentTime);
  id = glGetUniformLocation(currentShader, "sparksPerFirefly");
  glUniform1ui(id, sparksPerFirefly);
  id = glGetUniformLocation(currentShader, "noiseTexture");
  glUniform1i(id, 1); // texture unit 1 (noise)
  glDispatchCompute(ceil(1.0 * numSparks / sparksPerWorkGroup), 1, 1); // compensate for inability to use extension on my computer

  //  Wait for compute shader
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  ErrCheck("calculate spark positions (compute shader)");
}


// draw sparks around fireflies
void DrawFirefliesAndSparks(vec4 fireflyPositions[], int numSparks, int numFireflies) {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyPosBuf);
  vec4* fireflyPos = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_READ_BIT);
  // first, draw the fireflies
  mat4copy(modelView1, modelViewMat); // replacement for glPushMatrix()
  mat4copy(modelViewMat, fireflyModelViewMat); // use the same modelViewMatrix as when we created the fireflies
  unsigned int currentShader = simpleShader;
  glUseProgram(currentShader);
  for (int i = 0; i < numFireflies; ++i) {
    mat4copy(modelView2, modelViewMat); // replacement for glPushMatrix()
    // mat4translate(modelViewMat, fireflyPositions[i].x, fireflyPositions[i].y, fireflyPositions[i].z);
    mat4translate(modelViewMat, fireflyPos[i].x, fireflyPos[i].y, fireflyPos[i].z);
    mat4scale(modelViewMat, 0.05, 0.05, 0.05);
    PassMatricesToShader(currentShader, viewMat, modelViewMat, projectionMat);
    SimpleIcosahedron(currentShader); // represents a point light
    mat4copy(modelViewMat, modelView2); // replacement for glPopMatrix()
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); // free up the memory mapping to the firefly locations
  ErrCheck("fireflies draw");  
  
  /*
  ComputeSparkLocations();

  // PREPARE TO DRAW FIREFLY SPARKS using particle shader -- transparent stuff is drawn last
  // Example 16 is used as reference, and some code is borrowed.
  currentShader = particleShader;
  glUseProgram(currentShader); // this should be a particle shader
  //  Set particle size
  glPointSize(15); // use small particles
  // glPointSize(25);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, starTexture); // spark sprite
  glEnable(GL_POINT_SPRITE);
  glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glDepthMask(0); // disable z-buffer

  // prepare uniforms for particle shader
  id = glGetUniformLocation(currentShader, "sprite");
  glUniform1i(id, 0); // texture unit 0
  id = glGetUniformLocation(currentShader, "noiseTexture");
  glUniform1i(id, 1); // texture unit 1 (noise)

  // prepare vertex attributes
  // use positions calculated by compute shader
  glBindBuffer(GL_ARRAY_BUFFER, posbuf);
  id = glGetAttribLocation(currentShader, "Vertex");
  glVertexAttribPointer(id, 4, GL_FLOAT, 0, 16, (void*)0);
  glEnableVertexAttribArray(id);

  // use colors from the shader storage buffer
  glBindBuffer(GL_ARRAY_BUFFER, colbuf);
  id = glGetAttribLocation(currentShader, "Color");
  glVertexAttribPointer(id, 4, GL_FLOAT, 0, 16, (void*)0);
  glEnableVertexAttribArray(id);
  ErrCheck("set up to draw spark particles");

  // draw the sparks surrounding the fireflies
  PassMatricesToShader(currentShader, viewMat, modelViewMat, projectionMat);
  for (int i = 0; i < numFireflies; i++) {
    // set the location of the firefly who is the source of this spark
    mat4copy(modelView2, modelViewMat); // replacement for glPushMatrix()
    // mat4translate(modelViewMat, fireflyPositions[i].x, fireflyPositions[i].y, fireflyPositions[i].z);
    // mat4translate(modelViewMat, fireflyPos[i].x, fireflyPos[i].y, fireflyPos[i].z);
    // PassMatricesToShader(currentShader, viewMat, modelViewMat, projectionMat);
    glDrawArrays(GL_POINTS, i * sparksPerFirefly, sparksPerFirefly);
    mat4copy(modelViewMat, modelView2); // replacement for glPopMatrix()
    ErrCheck("draw firefly sparks loop");
  }

  // undo firefly settings
  glDisable(GL_POINT_SPRITE);
  glDisable(GL_BLEND);
  glDepthMask(1);
  id = glGetAttribLocation(currentShader, "Vertex");
  glDisableVertexAttribArray(id);
  id = glGetAttribLocation(currentShader, "Color");
  glDisableVertexAttribArray(id);
  glBindTexture(GL_TEXTURE_2D, blankTexture);
  ErrCheck("firefly sparks draw");
  */
  mat4copy(modelViewMat, modelView1); // replacement for glPopMatrix()
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
    ResetFireflies(firefliesPerSwarm, numFireflySwarms, numFireflies);
    ResetSparks(numSparks, fireflyColors);
}

float dot3(vec4 v1, vec4 v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec4 add3(vec4 v1, vec4 v2) {
  vec4 result = { .x = v1.x + v2.x, .y = v1.y + v2.y, .z = v1.z + v2.z, .w = 1.0 };
  return result;
}

vec4 scale3(float scalar, vec4 v) {
  vec4 result = { .x = scalar * v.x, .y = scalar * v.y, .z = scalar * v.z, .w = 1.0 };
  return result;
}

// element-wise multiplication
vec4 mult3(vec4 v1, vec4 v2) {
  vec4 result = { .x = v1.x * v2.x, .y = v1.y * v2.y, .z = v1.z * v2.z, .w = 1.0 };
  return result;
}

vec4 div3(vec4 v1, vec4 v2) {
  vec4 result = { .x = v1.x / v2.x, .y = v1.y / v2.y, .z = v1.z / v2.z, .w = 1.0 };
  return result;
}

float min(float a, float b) {
  if (a < b) {
    return a;
  }
  return b;
}

float max(float a, float b) {
  if (a > b) {
    return a;
  }
  return b;
}

vec4 min3(vec4 v1, vec4 v2) {
  vec4 result = { .x = min(v1.x, v2.x), .y = min(v1.y, v2.y), .z = min(v1.z, v2.z), .w = 1.0 };
  return result;
}

vec4 max3(vec4 v1, vec4 v2) {
  vec4 result = { .x = max(v1.x, v2.x), .y = max(v1.y, v2.y), .z = max(v1.z, v2.z), .w = 1.0 };
  return result;
}

vec4 min3Float(float scalar, vec4 v) {
  vec4 result = { .x = min(scalar, v.x), .y = min(scalar, v.y), .z = min(scalar, v.z), .w = 1.0 };
  return result;
}

vec4 max3Float(float scalar, vec4 v) {
  vec4 result = { .x = max(scalar, v.x), .y = max(scalar, v.y), .z = max(scalar, v.z), .w = 1.0 };
  return result;
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

  //  Set view
  ViewGL4(viewMat, th, ph, fov, dim);
  mat4copy(modelViewMat, viewMat);

  float position[] = { 3 * Cos(lTh), 1.0, 3 * Sin(lTh), 1.0 };
  /*
  // Place the light using shader 0 (simple shader)
  glUseProgram(simpleShader); // simple shader
  mat4copy(modelView1, modelViewMat); // replacement for glPushMatrix()
  // translate and scale down the icosahedron
  mat4translate(modelViewMat, position[0], position[1], position[2]);
  mat4scale(modelViewMat, 0.2, 0.2, 0.2);
  PassMatricesToShader(simpleShader, viewMat, modelViewMat, projectionMat);
  SimpleIcosahedron(simpleShader); // represents a point light
  mat4copy(modelViewMat, modelView1); // replacement for glPopMatrix()
  ErrCheck("Icosahedron light");
  */

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
  id = glGetUniformLocation(fireflyShader, "noiseTexture");
  glUniform1i(id, 1); // texture unit 1 (noise)

  vec4 fireflyPositions[maxFireflies];
  float fireflyPositionsFloat[maxFireflies * 4];
  vec4* fireflyPos;
  if (mode) {
    // NEW WAY TO CALCULATE FIREFLY POSITIONS (using Boids)
    ComputeFireflyLocations();
  }
  // copy firefly locations from the shader storage buffer to a C program array
  // unfortunately, the only way to pass the firefly locations as a uniform is to copy shader data to an array on the CPU, since glMapBufferRange data may not be used in OpenGL commands.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyPosBuf);
  fireflyPos = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_READ_BIT);
  for (int i = 0; i < numFireflies; ++i) {
    fireflyPositions[i] = fireflyPos[i];
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); // free up the memory mapping to the firefly locations
  fireflyPos = NULL;
  if (!mode) {
    // compute boids on CPU
    
    // first, get velocities
    vec4 fireflyVelocities[maxFireflies];
    vec4* fireflyVel;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyVelBuf);
    fireflyVel = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_READ_BIT);
    for (int i = 0; i < numFireflies; ++i) {
      fireflyVelocities[i] = fireflyVel[i];
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); // free up the memory mapping to the firefly locations
    fireflyVel = NULL;

    const float simulationSpeed = 2.0;
    float deltaTime = dt; // use global dt
    float dt = simulationSpeed * deltaTime; // shadow the global dt

    // related to separation (avoiding peers who are too close)
    const float hostileDistance = 0.05;
    const float hostileDistanceSquared = hostileDistance * hostileDistance;
    const float aversion = 0.5; // how much you want to avoid peers. This should be the strongest force if we don't want a super stable flock. We keep it low since we face aversion from multiple peers.
    const float aversionDecay = 5.0;

    // related to steering toward average heading
    const float localDistance = 0.5; // how close other members must be in order to be considered "flockmates"
    const float localDistanceSquared = localDistance * localDistance;
    const float targetSteeringTime = 1.5; // how many seconds it should take to match the average heading of the flockmates
    const float minSteerAcceleration = 1.0; // we need this to be larger than min cohesion speed so we focus on steering toward average heading once we are near the flock.
    const float maxSteerAcceleration = 1.5;

    // related to cohesion
    const float travelTimeToCenter = 1.0; // how many seconds you would like to take when trying to reach the center
    const float minCohesionSpeed = 0.5; // units per second
    const float maxCohesionSpeed = 3.0; // units per second

    // barrier to keep fireflies from escaping the scene
    const float barrierStrength = 0.01;

    for (int i = 0; i < numFireflies; ++i) {
      int gid = i;
      // initial positions
      vec4 p0 = fireflyPositions[gid];
      vec4 v0 = fireflyVelocities[gid];
      float speedLimit = 0.0; // use later with a differnt value
      float neighbors = 0.0; // use for separation and alignment.

      // https://en.wikipedia.org/wiki/Boids
      // 1. Separation: Avoid crowding local flockmates
      vec4 repulsion = { .x = 0.0, .y = 0.0, .z = 0.0, .w = 1.0 };
      for (int j = 0; j < numFireflies; ++j)
      {
        vec4 diff = add3(p0, scale3(-1, fireflyPositions[j])); // subtract pos[j] from p0
        float dist = dot3(diff, diff);
        float close = 0.0;
        if (hostileDistanceSquared <= dist) {
          close = 1.0;
        }
        neighbors += close;
        float scale = close * (aversion / (aversionDecay * dist + 1.0 - hostileDistanceSquared));
        repulsion = add3( repulsion, scale3(scale, diff) ); // if you are closer than hostile distance, this number grows larger than aversion
      }

      // 2. Alignment: Steer toward the average heading of local flockmates
      vec4 heading = { .x = 0.0, .y = 0.0, .z = 0.0, .w = 1.0 };
      neighbors = 0.0; // number of neighbors
      for (int j = 0; j < numFireflies; ++j)
      {
        vec4 diff = add3(p0, scale3(-1, fireflyPositions[j])); // subtract pos[i] from p0
        float dist = dot3(diff, diff);
        float close = 0.0;
        if (localDistanceSquared <= dist) {
          close = 1.0;
        }
        neighbors += close;
        heading = add3(heading, scale3(close, fireflyVelocities[j]));
      }
      heading = scale3(1 / neighbors, heading);

      vec4 steeringRequired = add3(heading, scale3(-1,  v0)); // heading - v0
      vec4 steeringAcceleration = scale3(1 / targetSteeringTime, steeringRequired);
      float steeringAccelerationMagnitude = sqrt(dot3(steeringAcceleration, steeringAcceleration)); // L2 norm of steeringAcceleration (length)
      speedLimit = min(steeringAccelerationMagnitude, maxSteerAcceleration) / steeringAccelerationMagnitude;
      steeringAcceleration = scale3(speedLimit, steeringAcceleration);
      speedLimit = max(steeringAccelerationMagnitude, minSteerAcceleration) / steeringAccelerationMagnitude;
      steeringAcceleration = scale3(speedLimit, steeringAcceleration);

      // 3. Cohesion: Calculate center of mass for cohesion
      vec4 com; // com = center of mass
      for (int j = 0; j < numFireflies; ++j)
      {
          com = add3(com, fireflyPositions[j]);
      }
      com = scale3(1 / numFireflies, com);
      vec4 cohesion = add3(com, scale3(-1, p0)); // com - p0
      vec4 cohesionVelocity = scale3(1 / travelTimeToCenter, cohesion); // note: this is really an acceleration. It's poorly named.
      float cohesionSpeed = sqrt(dot3(cohesionVelocity, cohesionVelocity)); // L2 norm (length)
      speedLimit = min(cohesionSpeed, maxCohesionSpeed) / cohesionSpeed;
      cohesionVelocity = scale3(speedLimit, cohesionVelocity);
      speedLimit = max(cohesionSpeed, minCohesionSpeed) / cohesionSpeed;
      cohesionVelocity = scale3(speedLimit, cohesionVelocity);

      // barrier to prevent excape from scene
      vec4 barrierDir = { .x = 0.0, .y = 1.5, .z = 0.0, .w = 1.0 };
      barrierDir = add3(barrierDir, scale3(-1, p0)); // barrierDir = the location above - p0
      vec4 barrierAcceleration = scale3(barrierStrength, scale3(sqrt(dot3(barrierDir, barrierDir)), barrierDir)); // barrierDir * length(barrierDir) * barrierStrength; this increases the barrier strength as distance increases

      vec4 v = add3(v0, scale3(dt, add3(repulsion, add3(steeringAcceleration, add3(cohesionVelocity, barrierAcceleration)))));
      vec4 p = add3(p0, scale3(dt, v));

      // save the results into the arrays
      fireflyPositions[gid] = p;
      fireflyPositions[gid] = v;
      fireflyPositionsFloat[4 * gid + 0] = p.x;
      fireflyPositionsFloat[4 * gid + 1] = p.y;
      fireflyPositionsFloat[4 * gid + 2] = p.z;
      fireflyPositionsFloat[4 * gid + 3] = 1.0;
    }

    // save the contents of the local arrays into the ones used by the compute shader in case we change modes later
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyPosBuf);
    fireflyPos = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for (int i = 0; i < numFireflies; ++i)
    {
        fireflyPos[i] = fireflyPositions[i];
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, fireflyVelBuf);
    fireflyVel = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numFireflies * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for (int i = 0; i < numFireflies; i++)
    {
        fireflyVel[i] = fireflyVelocities[i];
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  } // end of code to execute if mode is disabled

  // use the firefly shader and draw grass lit by fireflies
  glUseProgram(fireflyShader);
  mat4copy(fireflyModelViewMat, modelViewMat); // store the current modelViewMatrix so we can place the fireflies in the scene and remember their positions for later
  
  // pass the position of the fireflies for lighting purposes
  id = glGetUniformLocation(fireflyShader, "fireflyModelView");
  glUniformMatrix4fv(id, 1, 0, fireflyModelViewMat);
  id = glGetUniformLocation(fireflyShader, "fireflies");
  glUniform4fv(id, numFireflies, fireflyPositionsFloat);
  id = glGetUniformLocation(fireflyShader, "numFireflies");
  glUniform1i(id, numFireflies);
  // lTh is a convenient variable that changes with time. I can use it to oscillate through a portion of the 3D noise map in order to make the fireflies quake.
  id = glGetUniformLocation(fireflyShader, "t");
  glUniform1f(id, (float) lTh);
  ErrCheck("fireflies lighting");

  // draw the base plate
  PassMatricesToShader(fireflyShader, viewMat, modelViewMat, projectionMat);
  ErrCheck("before base plate");
  DrawBasePlate(fireflyShader, grassTexture);

  // Draw firefly sparks
  DrawFirefliesAndSparks(fireflyPositions, numSparks, numFireflies);

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
       mode = 1 - mode;
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
  GLFWwindow* window = InitWindow("John Salame HW10 - Compute Shader",0,600,600,&reshape,&key);

  //  Load shader
  simpleShader = CreateShaderProg("simple.vert", "simple.frag");
  fireflyShader = CreateShaderProg("firefly1.vert", "texture.frag");
  textureShader = CreateShaderProg("texture.vert", "texture.frag");
  particleShader = CreateShaderProg("particle.vert", "particle.frag");
  computeShader = CreateShaderProgCompute("spark.cs");
  boidsComputeShader = CreateShaderProgCompute("boids.cs");
  // shader[0] = CreateShaderProg("texture.vert", "blur.frag"); // filter
  // shader[1] = CreateShaderProg("texture.vert", "prewitt.frag"); // filter
  //  Load textures
  glActiveTexture(GL_TEXTURE0);
  tex = LoadTexBMP("pi.bmp");
  grassTexture = LoadTexBMP("grass.bmp");
  starTexture = LoadTexBMP("star.bmp");
  blankTexture = LoadTexBMP("blank.bmp");
  noiseTexture = CreateNoise3D(GL_TEXTURE1); // put noise in texture unit 1

  // Prepare for compute shader
  //  Get max workgroup size and count
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &ng);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &nw);
  if (ng > 8192) ng = 8192;
  maxParticles = nw * ng;
  printf("%d maximum work groups and %d maximum work group size.\n", ng, nw);

  // make the fireflies slightly yellow and transparent
  for (int i = 0; i < 4; ++i) {
      fireflyColors[4 * i + 0] = 0.8;
      fireflyColors[4 * i + 1] = 1.0; // greenish color
      fireflyColors[4 * i + 2] = 0.4;
      fireflyColors[4 * i + 3] = 0.6; // slightly transparent
  }
  // Initialize fireflies
  numFireflies = InitFireflies(firefliesPerSwarm, numFireflySwarms);
  printf("%d fireflies (%d per initial swarm, %d initial swarms\n", numFireflies, firefliesPerSwarm, numFireflySwarms);
  // Initialize things related to firefly sparks (for example, the compute shader and shader storage buffers)
  numSparks = InitSparks(sparksPerFirefly, numFireflies, fireflyColors);
  printf("%d sparks (%d per firefly)\n", numSparks, sparksPerFirefly);

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
