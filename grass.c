/**
 * John Salame
 * CSCI 5239 Advanced Computer Graphics
 * Create a blade of grass.
 */
#include "grass.h"
const int grassSize = 36;
const float grassData[] = {
// X     Y     Z    W       Nx   Ny   Nz     R    G    B    A      s    t
  // left face
  0.08, 0.8, -0.1, 1.0,   -1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.0,  0.0, -0.1, 1.0,   -1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.11, 0.9,  0.1, 1.0,   -1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.11, 0.9,  0.1, 1.0,   -1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.0,  0.0, -0.1, 1.0,   -1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.08, 0.0,  0.1, 1.0,   -1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,

  // front face
  0.11, 0.9,  0.1, 1.0,    0.0, 0.0, 1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.08, 0.0,  0.1, 1.0,    0.0, 0.0, 1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.24, 1.0,  0.1, 1.0,    0.0, 0.0, 1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.24, 1.0,  0.1, 1.0,    0.0, 0.0, 1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.08, 0.0,  0.1, 1.0,    0.0, 0.0, 1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.32, 0.0,  0.1, 1.0,    0.0, 0.0, 1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,

  // right face
  0.24, 1.0,  0.1, 1.0,    1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.32, 0.0,  0.1, 1.0,    1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.21, 0.9, -0.1, 1.0,    1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.21, 0.9, -0.1, 1.0,    1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.32, 0.0,  0.1, 1.0,    1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.24, 0.0, -0.1, 1.0,    1.0, 0.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,

  // back face
  0.21, 0.9, -0.1, 1.0,    0.0, 0.0, -1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.24, 0.0, -0.1, 1.0,    0.0, 0.0, -1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.08, 0.8, -0.1, 1.0,    0.0, 0.0, -1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.08, 0.8, -0.1, 1.0,    0.0, 0.0, -1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.24, 0.0, -0.1, 1.0,    0.0, 0.0, -1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.0,  0.0, -0.1, 1.0,    0.0, 0.0, -1.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,

  // top face
  0.08, 0.8, -0.1, 1.0,    0.0, 1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.11, 0.9,  0.1, 1.0,    0.0, 1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.21, 0.9, -0.1, 1.0,    0.0, 1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.21, 0.9, -0.1, 1.0,    0.0, 1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.11, 0.9,  0.1, 1.0,    0.0, 1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.24, 1.0,  0.1, 1.0,    0.0, 1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  
  // bottom face
  0.24, 0.0, -0.1, 1.0,    0.0, -1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.32, 0.0,  0.1, 1.0,    0.0, -1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.0,  0.0, -0.1, 1.0,    0.0, -1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.0,  0.0, -0.1, 1.0,    0.0, -1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.32, 0.0,  0.1, 1.0,    0.0, -1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
  0.08, 0.0,  0.1, 1.0,    0.0, -1.0, 0.0,   0.1, 0.4, 0.0, 1.0,   0.0, 0.0,
};

int getSizeOfGrassVBO(int numBlades) {
	return sizeof(grassData) * numBlades;
}
// for knowing how many vertices to render in glDrawArrays() in order to render a single blade of grass
int getNumRowsPerGrass() {
	return grassSize;
}
// copy a single blade of grass to the destination
void copyGrassData(float* dest, int numBlades) {
  int step = sizeof(grassData); // size of one blade of grass
  for (int i = 0; i < numBlades; i++) {
	memcpy(dest + step, grassData, sizeof(grassData));
  }
}

// For homework 4
void InitGrassVBO(float data[], int dataSize, unsigned int* grassVbo) {
  //  Get buffer name
  if (*grassVbo == 0)
    glGenBuffers(1, grassVbo);
  //  Bind VBO
  glBindBuffer(GL_ARRAY_BUFFER, *grassVbo);
  //  Copy cube to VBO
  glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
  //  Release VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  ErrCheck("grass VBO");
}

// For homework 4
void InitGrassVAO(int shader, unsigned int* grassVbo, unsigned int *grassVao) {
  // If we have already bound the vao to the vbo, then skip this function
  if (*grassVao != 0) {
		return;
  }
  //  Shader for which to get attibute locations
  glUseProgram(shader);

  //  Create cube VAO to bind attribute arrays
  glGenVertexArrays(1, grassVao);
  glBindVertexArray(*grassVao);

  //  Bind VBO
  glBindBuffer(GL_ARRAY_BUFFER, *grassVbo);
  //  Vertex
  int loc = glGetAttribLocation(shader, "Vertex");
  glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 52, (void*)0);
  glEnableVertexAttribArray(loc);
  //  Normal
  loc = glGetAttribLocation(shader, "Normal");
  glVertexAttribPointer(loc, 3, GL_FLOAT, 0, 52, (void*)16);
  glEnableVertexAttribArray(loc);
  //  Color
  loc = glGetAttribLocation(shader, "Color");
  glVertexAttribPointer(loc, 4, GL_FLOAT, 0, 52, (void*)28);
  glEnableVertexAttribArray(loc);
  //  Texture
  loc = glGetAttribLocation(shader, "Texture");
  glVertexAttribPointer(loc, 2, GL_FLOAT, 0, 52, (void*)44);
  glEnableVertexAttribArray(loc);

  //  Release VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //  Release VAO
  glBindVertexArray(0);
  //  Release shader
  glUseProgram(0);
  ErrCheck("grass VAO");
}

// OpenGL 2 Style
void bladeOfGrass(double height) {
  glPushMatrix();
  glTranslated(0.16, 0.0, 0.0);
  glScaled(1.0, height, 1.0);
  SetColor(0.1, 0.4, 0.0);
  glBegin(GL_QUAD_STRIP);
  // left face
  glVertex3f(-0.08, 0.8*height, -0.1);
  glVertex3f(-0.16, 0.0, -0.1);
  glVertex3f(-0.05, 0.9*height, 0.1);
  glVertex3f(-0.08, 0.0, 0.1);
  // front face
  glVertex3f(0.08, height, 0.1);
  glVertex3f(0.16, 0.0, 0.1);
  // right face
  glVertex3f(0.05, 0.9*height, -0.1);
  glVertex3f(0.08, 0.0, -0.1);
  // back face
  glVertex3f(-0.08, 0.8*height, -0.1);
  glVertex3f(-0.16, 0.0, -0.1);
  glEnd();
  
  glBegin(GL_QUADS);
  // top face
  glVertex3f(-0.08, 0.8*height, -0.1);
  glVertex3f(-0.05, 0.9*height, 0.1);
  glVertex3f(0.08, height, 0.1);
  glVertex3f(0.05, 0.9*height, -0.1);
  // bottom face
  glVertex3f(-0.16, 0.0, -0.1);
  glVertex3f(0.08, 0.0, -0.1);
  glVertex3f(0.16, 0.0, 0.1);
  glVertex3f(-0.08, 0.0, 0.1);
  glEnd();
  glPopMatrix();
}
