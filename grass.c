/**
 * John Salame
 * CSCI 5239 Advanced Computer Graphics
 * Create a blade of grass.
 */
#include "grass.h"

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