//  CSCIx239 library
//  Willem A. (Vlakkies) Schreuder
#include "CSCIx239.h"

//
//  Set projection matrix
//
void Projection(float fov,float asp,float dim)
{
   //  Set projection matrix
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   //  Perspective transformation
   if (fov)
      gluPerspective(fov,asp,dim/16,16*dim);
   //  Orthogonal transformation
   else
      glOrtho(-asp*dim,asp*dim,-dim,+dim,-dim,+dim);
   //  Reset modelview
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

//
//  Set (Model)View Matrix
//
void View(float th,float ph,float fov,float dim)
{
   //  Set ModelView matrix
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   //  Perspective - set eye position
   if (fov)
   {
      float Ex = -2*dim*Sin(th)*Cos(ph);
      float Ey = +2*dim        *Sin(ph);
      float Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
}

// pass in the projection matrix
void ProjectionGL4(float modelViewMat[], float projectionMat[], float fov, float asp, float dim) {
  // reset projection matrix
  mat4identity(projectionMat);
  float zNear = dim / 16;
  float zFar = 16 * dim;
  if (fov) {
    mat4perspective(projectionMat, fov, asp, zNear, zFar);
  }
  else {
    mat4ortho(projectionMat, -asp * dim, asp * dim, -dim, +dim, -dim, +dim);
  }
  // reset modelViewMatrix
  mat4identity(modelViewMat);
}

void ViewGL4(float modelViewMat[], float th, float ph, float fov, float dim) {
    mat4identity(modelViewMat);
    if (fov) {
      float Ex = -2 * dim * Sin(th) * Cos(ph);
      float Ey = +2 * dim * Sin(ph);
      float Ez = +2 * dim * Cos(th) * Cos(ph);
      gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
      mat4lookAt(modelViewMat, Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
    }
    else {
        mat4rotate(modelViewMat, ph, 1, 0, 0);
        mat4rotate(modelViewMat, th, 0, 1, 0);
    }
}
