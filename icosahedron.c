//  CSCIx239 library
//  Willem A. (Vlakkies) Schreuder
#include "CSCIx239.h"

//  Icosahedron data stored in VBO
static unsigned int vbo=0;
static unsigned int vao=0;
static const int n      = 60;                //  Number of vertexes
static const int stride = 11*sizeof(float);  //  Stride (bytes)
//  Vertex coordinates, normals, textures and colors
static const float data[] =
{
// X      Y      Z       Nx     Ny     Nz    S   T   R G B 
 0.276, 0.851, 0.447,  0.471, 0.342, 0.761, 0.0,0.0, 0,0,1,
 0.894, 0.000, 0.447,  0.471, 0.342, 0.761, 1.0,0.0, 0,0,1,
 0.000, 0.000, 1.000,  0.471, 0.342, 0.761, 0.5,1.0, 0,0,1,
-0.724, 0.526, 0.447, -0.180, 0.553, 0.761, 0.0,0.0, 0,1,0,
 0.276, 0.851, 0.447, -0.180, 0.553, 0.761, 1.0,0.0, 0,1,0,
 0.000, 0.000, 1.000, -0.180, 0.553, 0.761, 0.5,1.0, 0,1,0,
-0.724,-0.526, 0.447, -0.582, 0.000, 0.762, 0.0,0.0, 0,1,1,
-0.724, 0.526, 0.447, -0.582, 0.000, 0.762, 1.0,0.0, 0,1,1,
 0.000, 0.000, 1.000, -0.582, 0.000, 0.762, 0.5,1.0, 0,1,1,
 0.276,-0.851, 0.447, -0.180,-0.553, 0.761, 0.0,0.0, 1,0,1,
-0.724,-0.526, 0.447, -0.180,-0.553, 0.761, 1.0,0.0, 1,0,1,
 0.000, 0.000, 1.000, -0.180,-0.553, 0.761, 0.5,1.0, 1,0,1,
 0.894, 0.000, 0.447,  0.471,-0.342, 0.761, 0.0,0.0, 1,1,0,
 0.276,-0.851, 0.447,  0.471,-0.342, 0.761, 1.0,0.0, 1,1,0,
 0.000, 0.000, 1.000,  0.471,-0.342, 0.761, 0.5,1.0, 1,1,0,
 0.000, 0.000,-1.000,  0.180, 0.553,-0.761, 0.0,0.0, 0,0,1,
 0.724, 0.526,-0.447,  0.180, 0.553,-0.761, 1.0,0.0, 0,0,1,
-0.276, 0.851,-0.447,  0.180, 0.553,-0.761, 0.5,1.0, 0,0,1,
 0.000, 0.000,-1.000, -0.471, 0.342,-0.761, 0.0,0.0, 0,1,0,
-0.276, 0.851,-0.447, -0.471, 0.342,-0.761, 1.0,0.0, 0,1,0,
-0.894, 0.000,-0.447, -0.471, 0.342,-0.761, 0.5,1.0, 0,1,0,
 0.000, 0.000,-1.000, -0.471,-0.342,-0.761, 0.0,0.0, 0,1,1,
-0.894, 0.000,-0.447, -0.471,-0.342,-0.761, 1.0,0.0, 0,1,1,
-0.276,-0.851,-0.447, -0.471,-0.342,-0.761, 0.5,1.0, 0,1,1,
 0.000, 0.000,-1.000,  0.180,-0.553,-0.761, 0.0,0.0, 1,0,0,
-0.276,-0.851,-0.447,  0.180,-0.553,-0.761, 1.0,0.0, 1,0,0,
 0.724,-0.526,-0.447,  0.180,-0.553,-0.761, 0.5,1.0, 1,0,0,
 0.000, 0.000,-1.000,  0.582, 0.000,-0.762, 0.0,0.0, 1,0,1,
 0.724,-0.526,-0.447,  0.582, 0.000,-0.762, 1.0,0.0, 1,0,1,
 0.724, 0.526,-0.447,  0.582, 0.000,-0.762, 0.5,1.0, 1,0,1,
 0.894, 0.000, 0.447,  0.761, 0.552, 0.180, 0.0,0.0, 1,1,0,
 0.276, 0.851, 0.447,  0.761, 0.552, 0.180, 1.0,0.0, 1,1,0,
 0.724, 0.526,-0.447,  0.761, 0.552, 0.180, 0.5,1.0, 1,1,0,
 0.276, 0.851, 0.447, -0.291, 0.894, 0.179, 0.0,0.0, 0,0,1,
-0.724, 0.526, 0.447, -0.291, 0.894, 0.179, 1.0,0.0, 0,0,1,
-0.276, 0.851,-0.447, -0.291, 0.894, 0.179, 0.5,1.0, 0,0,1,
-0.724, 0.526, 0.447, -0.940, 0.000, 0.179, 0.0,0.0, 0,1,0,
-0.724,-0.526, 0.447, -0.940, 0.000, 0.179, 1.0,0.0, 0,1,0,
-0.894, 0.000,-0.447, -0.940, 0.000, 0.179, 0.5,1.0, 0,1,0,
-0.724,-0.526, 0.447, -0.291,-0.894, 0.179, 0.0,0.0, 0,1,1,
 0.276,-0.851, 0.447, -0.291,-0.894, 0.179, 1.0,0.0, 0,1,1,
-0.276,-0.851,-0.447, -0.291,-0.894, 0.179, 0.5,1.0, 0,1,1,
 0.276,-0.851, 0.447,  0.761,-0.552, 0.180, 0.0,0.0, 1,0,0,
 0.894, 0.000, 0.447,  0.761,-0.552, 0.180, 1.0,0.0, 1,0,0,
 0.724,-0.526,-0.447,  0.761,-0.552, 0.180, 0.5,1.0, 1,0,0,
 0.276, 0.851, 0.447,  0.291, 0.894,-0.179, 0.0,0.0, 1,0,1,
-0.276, 0.851,-0.447,  0.291, 0.894,-0.179, 1.0,0.0, 1,0,1,
 0.724, 0.526,-0.447,  0.291, 0.894,-0.179, 0.5,1.0, 1,0,1,
-0.724, 0.526, 0.447, -0.761, 0.552,-0.180, 0.0,0.0, 1,1,0,
-0.894, 0.000,-0.447, -0.761, 0.552,-0.180, 1.0,0.0, 1,1,0,
-0.276, 0.851,-0.447, -0.761, 0.552,-0.180, 0.5,1.0, 1,1,0,
-0.724,-0.526, 0.447, -0.761,-0.552,-0.180, 0.0,0.0, 0,0,1,
-0.276,-0.851,-0.447, -0.761,-0.552,-0.180, 1.0,0.0, 0,0,1,
-0.894, 0.000,-0.447, -0.761,-0.552,-0.180, 0.5,1.0, 0,0,1,
 0.276,-0.851, 0.447,  0.291,-0.894,-0.179, 0.0,0.0, 0,1,0,
 0.724,-0.526,-0.447,  0.291,-0.894,-0.179, 1.0,0.0, 0,1,0,
-0.276,-0.851,-0.447,  0.291,-0.894,-0.179, 0.5,1.0, 0,1,0,
 0.894, 0.000, 0.447,  0.940, 0.000,-0.179, 0.0,0.0, 0,1,1,
 0.724, 0.526,-0.447,  0.940, 0.000,-0.179, 1.0,0.0, 0,1,1,
 0.724,-0.526,-0.447,  0.940, 0.000,-0.179, 0.5,1.0, 0,1,1,
};

// Addition for GL4, unlit, untextured
void SimpleIcosahedron(unsigned int shader) {
  //  Initialize VBO on first use
  if (!vbo)
  {
      //  Get buffer name
      glGenBuffers(1, &vbo);
      //  Bind VBO
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      //  Copy icosahedron to VBO
      glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  }
  //  On subsequanet calls, just bind VBO
  else
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // Initialize the VAO on first use
  if (!vao) {
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao); // use the VAO
      glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind VBO
      int loc = glGetAttribLocation(shader, "Vertex");
      glVertexAttribPointer(loc, 3, GL_FLOAT, 0, 44, (void*)0);
      glEnableVertexAttribArray(loc);
  }

  glBindVertexArray(vao);
  //  Draw icosahedron
  glBindVertexArray(vao); // use VAO
  glDrawArrays(GL_TRIANGLES, 0, n);
  //  Release VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //  Release VAO
  glBindVertexArray(0);
  ErrCheck("simple icosahedron");
}
void CleanupIcosahedron() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vbo);
}

//
//  Solid unit icosahedron
//
void SolidIcosahedron()
{
   //  Initialize VBO on first use
   if (!vbo)
   {
      //  Get buffer name
      glGenBuffers(1,&vbo);
      //  Bind VBO
      glBindBuffer(GL_ARRAY_BUFFER,vbo);
      //  Copy icosahedron to VBO
      glBufferData(GL_ARRAY_BUFFER,sizeof(data),data,GL_STATIC_DRAW);
   }
   //  On subsequanet calls, just bind VBO
   else
      glBindBuffer(GL_ARRAY_BUFFER,vbo);

   //  Define arrays
   glVertexPointer(3,GL_FLOAT,stride,(void*)0);
   glNormalPointer(GL_FLOAT,stride,(void*)12);
   glTexCoordPointer(2,GL_FLOAT,stride,(void*)24);
   glColorPointer(3,GL_FLOAT,stride,(void*)32);

   //  Enable arrays
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   //  Draw icosahedron
   glDrawArrays(GL_TRIANGLES,0,n);

   //  Disable arrays
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   //  Release VBO
   glBindBuffer(GL_ARRAY_BUFFER,0);
}

//
//  Textured icosahedron
//
void TexturedIcosahedron(int tex)
{
   //  Draw with texture
   if (tex)
   {
      glBindTexture(GL_TEXTURE_2D,tex);
      glEnable(GL_TEXTURE_2D);
      SolidIcosahedron();
      glDisable(GL_TEXTURE_2D);
   }
   //  Draw without texture
   else
      SolidIcosahedron();
}

//
//  General icosahedron
//
void Icosahedron(float x,float y,float z , float r, float th,float ph , int tex)
{

   //  Transform
   glPushMatrix();
   glTranslated(x,y,z);
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);
   glScaled(r,r,r);
   //  Draw textured cube
   TexturedIcosahedron(tex);
   // Restore
   glPopMatrix();
}
