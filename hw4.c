/*
 * John Salame
 * CSCI 5239 Advanced Computer Graphics
 * Homework 4
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
int shader[NUM_SHADERS];  //  Shader
float asp=1;   //  Aspect ratio
float dim=3;   //  Size of world
float objX = 0;
float objY = 0;
const char* text[NUM_SHADERS] = {"Simple Shader","Fireflies 1"};
int numPatches = 3;
int oldNumPatches = 0;
double* grassHeights = 0; // will be a 2d array of grass heights
float* grassDataMain = 0;
unsigned int grassVbo = 0;
unsigned int grassVao = 0;

// OPENGL4 STUFF
float modelViewMat[16];
float projectionMat[16];
float modelView1[16]; // temporary solution to glPushMatrix()
float modelView2[16]; // temporary solution to glPushMatrix()


// Helper function for modifying 
void PassMatricesToShader(int shaderProgram, float modelViewMat[], float projectionMat[]) {
  float modelViewProjectionMat[16];
  float normalMat[9];
  mat4identity(modelViewProjectionMat);
  // modelViewProjectionMat = projectionMat * modelViewMat
  mat4multMatrix(modelViewProjectionMat, projectionMat);
  mat4multMatrix(modelViewProjectionMat, modelViewMat);
  // calculate normal matrix from ModelViewMatrix
  mat4normalMatrix(modelViewMat, normalMat);
  int id = glGetUniformLocation(shaderProgram, "ModelViewMatrix");
  glUniformMatrix4fv(id, 1, 0, modelViewMat);
  id = glGetUniformLocation(shaderProgram, "ModelViewProjectionMatrix");
  glUniformMatrix4fv(id, 1, 0, modelViewProjectionMat);
  id = glGetUniformLocation(shaderProgram, "NormalMatrix");
  glUniformMatrix3fv(id, 1, 0, normalMat);
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
  //  Set material and lighting interaction (not sure if this matters outside of fixed pipeline)
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL); // without this enabled, glColor4fv does not apply, but the materials do
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // mix colors

  //  Set view
  // View(th, ph, fov, dim);
  ViewGL4(modelViewMat, th, ph, fov, dim);

  /*
  //code related to lighting
  // Don't light the light source; use program 0 (fixed pipeline) so lighting enable/disable has meaning.
  glUseProgram(0);
  float ambient = 0.2;
  float diffuse = 0.5;
  float specular = 0.3;
  float lTh = 0.0;
  float l0Position[] = { dim * Cos(lTh), 0.0, -dim * Sin(lTh), 1.0 };
  glShadeModel(GL_SMOOTH);
  // draw the light
  Lighting(l0Position[0], l0Position[1], l0Position[2], ambient, diffuse, specular);
  */

  /*
  // Enable shader and draw object
  if (mode)
  {
    glUseProgram(shader[mode]);
    //int id = glGetUniformLocation(shader,"time");
    //glUniform1f(id,glfwGetTime());
    int id = glGetUniformLocation(shader[mode], "dim");
    glUniform1f(id, 0.5); // squares of side length 50
  }
  */

  // set up the fireflies
  GLfloat firefly1[4] = { 1.0, 1.0, 2.0, 1.0 };
  GLfloat firefly2[4] = { -1.0, 1.0, 1.0, 1.0 };
  GLfloat firefly3[4] = { 0.0, 1.0, -2.0, 1.0 };
  GLfloat firefly4[4] = { 0.5, 0.5, 0.0, 1.0 };
  // add a bit of noise
  /*
  for (int i = 1; i < 3; i++) {
    firefly1[i] += 0.1 * (rand() % 5 - 2);
    firefly2[i] += 0.1 * (rand() % 5 - 2);
    firefly3[i] += 0.1 * (rand() % 5 - 2);
    firefly4[i] += 0.1 * (rand() % 5 - 2);
  }
  */
  // draw the fireflies
  float fireflyModelViewMat[16];
  mat4copy(fireflyModelViewMat, modelViewMat); // store the current modelViewMatrix so we can place the fireflies in the scene and remember their positions for later
  glUseProgram(shader[0]); // draw fireflies with the simplest shader
  Sphere(firefly1[0], firefly1[1], firefly1[2], 0.1, 0, 8, 0);
  Sphere(firefly2[0], firefly2[1], firefly2[2], 0.1, 0, 8, 0);
  Sphere(firefly3[0], firefly3[1], firefly3[2], 0.1, 0, 8, 0);
  Sphere(firefly4[0], firefly4[1], firefly4[2], 0.1, 0, 8, 0);
  // use the firefly shader
  mode = 1;
  glUseProgram(shader[mode]);
  // pass the position of the fireflies
  glLightfv(GL_LIGHT0, GL_POSITION, firefly1);
  glLightfv(GL_LIGHT1, GL_POSITION, firefly2);
  glLightfv(GL_LIGHT2, GL_POSITION, firefly3);
  glLightfv(GL_LIGHT3, GL_POSITION, firefly4);
  ErrCheck("fireflies");

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
    int numBlades = 10*numPatches*6*numPatches;
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
    // TO-DO: modify the array based on grass height
    InitGrassVBO(grassDataMain, grassVboSize, &grassVbo);
    printf("VBO: %d\n", grassVbo);
    InitGrassVAO(shader[mode], &grassVbo, &grassVao);
    printf("VAO: %d\n", grassVao);
    ErrCheck("grass main");
  }


  // Now draw the lawn

  glUseProgram(shader[mode]);
  //  Bind attribute arrays using VAO (VAO knows VBO to use)
  glBindVertexArray(grassVao);
  int grassVertices = getNumRowsPerGrass(); // number of vertices per blade of grass
  int drawStart = 0; // offset to draw from in glDrawArrays()

  mat4copy(modelView1, modelViewMat); // replacement for glPushMatrix()
  mat4translate(modelViewMat, 0.0, 0.0, -numPatches); // start at the back row
  for(int i = 0; i < 10*numPatches; i++) {
    // double* rowptr = grassHeights + i*6*numPatches; // add the current row * the length of the rows
    mat4copy(modelView2, modelViewMat);
    mat4translate(modelViewMat, -numPatches, 0.0, 0.0); // start of row (left side)
    // travserse the row
    for (int j = 0; j < 6*numPatches; j++) {
      PassMatricesToShader(shader[mode], modelViewMat, projectionMat);
      // bladeOfGrass(rowptr[j]);
      glDrawArrays(GL_TRIANGLES, drawStart, grassVertices);
      mat4translate(modelViewMat, 0.30, 0.0, 0.0);
      drawStart += grassVertices;
    }
    mat4copy(modelViewMat, modelView2); // replacement for glPopMatrix()
    mat4translate(modelViewMat, 0.0, 0.0, 0.2); // change the row
  }
  mat4copy(modelViewMat, modelView1); // replacement for glPopMatrix()

  //  Release attribute arrays
  glBindVertexArray(0);

  //  Revert to fixed pipeline
  glUseProgram(0);
  //  Display axes
  Axes(2);
  //  Display parameters
  SetColor(1,1,1);
  glWindowPos2i(5,5);
  int fps = FramesPerSecond();
  Print("Angle=%d,%d  Dim=%.1f Projection=%s Mode=%s Frames Per Second=%d",th,ph,dim,fov>0?"Perpective":"Orthogonal",text[mode],fps);
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
       th = ph = 0;
       objX = 0; objY = 0;
   }
   //  Switch shaders
   else if (key==GLFW_KEY_M)
      mode = (mode + 1) % NUM_SHADERS;
   //  Switch objects
   else if (key==GLFW_KEY_O)
      obj = 1-obj;
   //  Switch between perspective/orthogonal
   else if (key==GLFW_KEY_P)
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
}

//
//  Main program with GLFW event loop
//
int main(int argc,char* argv[])
{
   //  Initialize GLFW
   GLFWwindow* window = InitWindow("John Salame HW 3 - Performance",0,600,600,&reshape,&key);

   //  Load shader
   shader[0] = CreateShaderProg("simple.vert", "simple.frag");
   shader[1] = CreateShaderProg("firefly1.vert", "simple.frag");
   //  Load textures
   tex = LoadTexBMP("pi.bmp");

   //  Event loop
   ErrCheck("init");
   glEnable(GL_CULL_FACE);
   while(!glfwWindowShouldClose(window))
   {
      //  Display
      display(window);
      //  Process any events
      glfwPollEvents();
   }
   //  Shut down GLFW
   glfwDestroyWindow(window);
   glfwTerminate();
   return 0;
}