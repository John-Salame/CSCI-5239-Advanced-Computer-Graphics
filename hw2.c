/*
 *  John Salame
 * Homework 1
 * Adapted from Example 01 as starter code
 *
 *  Key bindings:
 *  m          Toggle shader
 *  o          Change objects
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx239.h"
int mode=1;    //  Shader
int th=0,ph=0; //  View angles
int fov=57;    //  Field of view (for perspective)
int tex=0;     //  Texture
int obj=0;     //  Object
int shader=0;  //  Shader
float asp=1;   //  Aspect ratio
float dim=3;   //  Size of world
float objX = 0;
float objY = 0;
const char* text[] = {"Fixed Pipeline","NDC Shader"};

//
//  Refresh display
//
void display(GLFWwindow* window)
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Set view
   View(th,ph,fov,dim);
   glTranslatef(objX, objY, 0.0);

   //  Enable shader
   if (mode)
   {
     glUseProgram(shader);
     //int id = glGetUniformLocation(shader,"time");
     //glUniform1f(id,glfwGetTime());
     int id = glGetUniformLocation(shader, "dim");
     glUniform1f(id,0.5); // squares of side length 50
   }
   else
     glUseProgram(0);
   //  Draw scene
   if (obj)
     TexturedIcosahedron(tex);
   else
     TexturedSphere(24, tex);
     //TexturedCube(tex);
   //  Revert to fixed pipeline
   glUseProgram(0);

   //  Display axes
   Axes(2);
   //  Display parameters
   SetColor(1,1,1);
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Projection=%s Mode=%s",th,ph,dim,fov>0?"Perpective":"Orthogonal",text[mode]);
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
   // top left corner
   else if (key == GLFW_KEY_1) {
       objX = -dim; objY = dim;
   }
   // bottom left corner
   else if (key == GLFW_KEY_2) {
       objX = -dim; objY = -dim;
   }
   // top right corner
   else if (key == GLFW_KEY_3) {
       objX = dim; objY = dim;
   }
   // bottom right corner
   else if (key == GLFW_KEY_4) {
       objX = dim; objY = -dim;
   }
   //  Switch shaders
   else if (key==GLFW_KEY_M)
      mode = 1-mode;
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

   //  Wrap angles
   th %= 360;
   ph %= 360;
   //  Update projection
   Projection(fov,asp,dim);
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
   Projection(fov,asp,dim);
}

//
//  Main program with GLFW event loop
//
int main(int argc,char* argv[])
{
   //  Initialize GLFW
   GLFWwindow* window = InitWindow("John Salame HW 1",1,600,600,&reshape,&key);

   //  Load shader
   shader = CreateShaderProg(NULL,"sawtooth.frag");
   //  Load textures
   tex = LoadTexBMP("pi.bmp");

   //  Event loop
   ErrCheck("init");
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
