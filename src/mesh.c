/* mesh.c
**
**    code for the display of the mesh 3d-models
**
**    foobillard++ started at 12/2010
**    Copyright (C) 2010 - 2013 Holger Schaekel (foobillardplus@go4more.de)
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License Version 2 as
**    published by the Free Software Foundation;
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**/

#include <stdio.h>
#include <stdlib.h>
#if defined(__APPLE__) && !defined(__MOBILE__)
 #include <OpenGL/glu.h>
 #include <OpenGL/gl.h>
#else
 #include <GL/glu.h>
 #include <GL/gl.h>
#endif
#include "barchair.h"
#include "bartable.h"
#include "burlap_sofa.h"
#include "fireplace.h"
#include "png_loader.h"
#include "fire.h"

// only define, if OpenGL version is higher then 1.5 <-- most netbooks/notebooks/tablets are incompatible (1.4)
//#define USE_BINDBUFFER

#ifdef USE_BINDBUFFER
  GLuint vbo,vbo1,vbo2,vbo2a,vbo3,vbo4;      // the vertex obj for sofa, chair, table, camin
  GLuint vinx,vinx1,vinx2,vinx2a,vinx3,vinx4;  // the texture ob for sofa, chair, table, camin
#endif
  GLuint sofatexbind = 0;
  GLuint chairtexbind = 0;
  GLuint camintexbind = 0;

// gl-compile id's for the meshes
  int sofa_id= -1;         // glcompile-id sofa
  int chair_id= -1;        // glcompile-id chair
  int bartable_id= -1;     // glcompile-id bar-table
  int camin_id= -1;        // glcompile-id camin

/***********************************************************************
 *          drawing of the meshes (array_buffer)                       *
 ***********************************************************************/

#ifdef USE_BINDBUFFER

#define BUFFER_OFFSET(x)((char *)NULL+(x))

void DrawMesh(GLuint _vbo, GLuint _inx,  GLsizei count)
{
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _inx);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof (struct vertex_struct), BUFFER_OFFSET(0));

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof (struct vertex_struct), BUFFER_OFFSET(3 * sizeof (float)));

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof (struct vertex_struct), BUFFER_OFFSET(6 * sizeof(float)));

  glDrawElements(GL_TRIANGLES, count * 3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

#else

/***********************************************************************
 *              drawing of the meshes (standard)                       *
 ***********************************************************************/

void DrawMesh(GLsizei count, char *vertexbegin, char *indexbegin)
{

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof (struct vertex_struct), vertexbegin);

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof (struct vertex_struct), &vertexbegin[(3 * sizeof (float))]);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof (struct vertex_struct), &vertexbegin[(6 * sizeof(float))]);

  glDrawElements(GL_TRIANGLES, count * 3, GL_UNSIGNED_SHORT, indexbegin);

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

#endif

/***********************************************************************
 *             for every furniture one function                        *
 ***********************************************************************/

void MakeSofa(void)
{
   if(sofa_id != -1) {
     glDeleteLists(sofa_id,1);
   }
   sofa_id = glGenLists(1);
   glNewList(sofa_id, GL_COMPILE);
   glBindTexture(GL_TEXTURE_2D,sofatexbind);
#ifdef USE_BINDBUFFER
   DrawMesh(vbo, vinx, FACES_COUNT);
#else
   DrawMesh(FACES_COUNT, (char *)&vertexs, (char *)&indexes);
#endif
   glEndList();
}

void MakeChair(void)
{
   if(chair_id != -1) {
     glDeleteLists(chair_id,1);
   }
   chair_id = glGenLists(1);
   glNewList(chair_id, GL_COMPILE);
   glBindTexture(GL_TEXTURE_2D,chairtexbind);
#ifdef USE_BINDBUFFER
   DrawMesh(vbo1, vinx1, FACES1_COUNT);
#else
   DrawMesh(FACES1_COUNT, (char *)&vertexs1, (char *)indexes1);
#endif
   glEndList();
}

void MakeTable(void)
{
	  static GLfloat ambient_torus[3] = {0.19, 0.19, 0.19};		// Torus
	  static GLfloat diffuse_torus[3] = {0.51, 0.51, 0.51};
	  static GLfloat specular_torus[3]= {0.51, 0.51, 0.51};

   if(bartable_id != -1) {
     glDeleteLists(bartable_id,1);
   }
   bartable_id = glGenLists(1);
   glNewList(bartable_id, GL_COMPILE);
   glDisable(GL_TEXTURE_2D);
  	glMaterialfv(GL_FRONT,GL_AMBIENT, ambient_torus);
  	glMaterialfv(GL_FRONT,GL_DIFFUSE, diffuse_torus);
  	glMaterialfv(GL_FRONT,GL_SPECULAR, specular_torus);
  	glMaterialf (GL_FRONT, GL_SHININESS, 51);

#ifdef USE_BINDBUFFER
	  DrawMesh(vbo2, vinx2, FACES2_COUNT);
#else
	  DrawMesh(FACES2_COUNT, (char *)&vertexs2, (char *)&indexes2);
#endif
   glEnable (GL_BLEND);
   glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR); // upper glass)
#ifdef USE_BINDBUFFER
	  DrawMesh(vbo2a, vinx2a, FACES2a_COUNT);
#else
	  DrawMesh(FACES2a_COUNT, (char *)&vertexs2a, (char *)&indexes2a);
#endif
   glDisable (GL_BLEND);
   glEnable(GL_TEXTURE_2D);
   glEndList();
}

void MakeCamin(void)
{
   if(camin_id != -1) {
     glDeleteLists(camin_id,1);
   }
   camin_id = glGenLists(1);
   glNewList(camin_id, GL_COMPILE);
   glBindTexture(GL_TEXTURE_2D,camintexbind);
#ifdef USE_BINDBUFFER
   DrawMesh(vbo3, vinx3, FACES3_COUNT);
#else
   DrawMesh(FACES3_COUNT, (char *)&vertexs3, (char *)&indexes3);
#endif
   glEndList();
}

/***********************************************************************
 *                    Initialization for the meshes                    *
 ***********************************************************************/

void InitMesh(void) {
  // free texbinds
  if(sofatexbind) {
    glDeleteTextures(1,&sofatexbind);
  }
  if(chairtexbind) {
    glDeleteTextures(1,&chairtexbind);
  }
  if(camintexbind) {
    glDeleteTextures(1,&camintexbind);
  }
   //sofa
   fprintf(stderr,"Initialize default mesh-graphics\n");
   create_png_texbind("sofa.png", &sofatexbind, 3, GL_RGB);
#ifdef USE_BINDBUFFER
   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof (struct vertex_struct) * VERTEX_COUNT, vertexs, GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenBuffers(1, &vinx);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vinx);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indexes[0]) * FACES_COUNT * 3, indexes, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
   //chair
   create_png_texbind("full_symbol.png", &chairtexbind, 4, GL_RGBA);
#ifdef USE_BINDBUFFER
   glGenBuffers(1, &vbo1);
   glBindBuffer(GL_ARRAY_BUFFER, vbo1);
   glBufferData(GL_ARRAY_BUFFER, sizeof (struct vertex_struct) * VERTEX1_COUNT, vertexs1, GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenBuffers(1, &vinx1);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vinx1);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indexes1[0]) * FACES1_COUNT * 3, indexes1, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
   //bar table
#ifdef USE_BINDBUFFER
   glGenBuffers(1, &vbo2);
   glBindBuffer(GL_ARRAY_BUFFER, vbo2);
   glBufferData(GL_ARRAY_BUFFER, sizeof (struct vertex_struct) * VERTEX2_COUNT, vertexs2, GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenBuffers(1, &vinx2);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vinx2);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indexes2[0]) * FACES2_COUNT * 3, indexes2, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   glGenBuffers(1, &vbo2a);
   glBindBuffer(GL_ARRAY_BUFFER, vbo2a);
   glBufferData(GL_ARRAY_BUFFER, sizeof (struct vertex_struct) * VERTEX2a_COUNT, vertexs2a, GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenBuffers(1, &vinx2a);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vinx2a);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indexes2a[0]) * FACES2a_COUNT * 3, indexes2a, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
   //camin
   create_png_texbind("camin.png", &camintexbind, 4, GL_RGBA);
#ifdef USE_BINDBUFFER
   glGenBuffers(1, &vbo3);
   glBindBuffer(GL_ARRAY_BUFFER, vbo3);
   glBufferData(GL_ARRAY_BUFFER, sizeof (struct vertex_struct) * VERTEX3_COUNT, vertexs3, GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenBuffers(1, &vinx3);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vinx3);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indexes3[0]) * FACES3_COUNT * 3, indexes3, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
  fprintf(stderr,"Initialize default Sofa object\n");
  MakeSofa();
  fprintf(stderr,"Initialize default Chair object\n");
  MakeChair();
  fprintf(stderr,"Initialize default Table object\n");
  MakeTable();
  fprintf(stderr,"Initialize default Fireplace object\n");
  MakeCamin();
  fprintf(stderr,"Initialize fire object\n");
  init_fire();  //initialize the fireflame for the fireplace
}

