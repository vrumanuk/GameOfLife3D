////////////////////////////////////////////////////
// GameOfLife3D.cpp
//
// Projects the game of life onto three dimensional objects
//
// Nathan Berkley
// Steven Cropp
// Viktor Rumanuk
////////////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <cmath>
#include <limits.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

const int X_MAX = 100;
const int Y_MAX = 100;

unsigned int dragState = 0;
bool isStepping = 0;
unsigned char activeBoard = 0;
unsigned char mode = 0;

// Wrap modes
unsigned char xWrapMode = 0;
unsigned char yWrapMode = 0;
// 0 is no wrapping
// 1 is normal wrapping
// 2 is sphere vertex wrapping

unsigned int stepPeriod = 400;
unsigned int maxStepPeriod = 5000;
unsigned int minStepPeriod = 50;

unsigned char currentRed = 128;
unsigned char currentGreen = 128;
unsigned char currentBlue = 128;

unsigned char cursorRed = 128;
unsigned char cursorGreen = 128;
unsigned char cursorBlue = 128;

// ADD A WAY FOR USERS TO CHANGE THIS
unsigned char minimumRevive = 3;
unsigned char minimumSurvive = 2;
unsigned char maximumForLife = 3;

unsigned char colors[2][X_MAX][Y_MAX][3];
unsigned char* tex = NULL;
static unsigned int texture;

int windowWidth = 600;
int windowHeight = 600;
unsigned int xSquares = 16;
unsigned int ySquares = 16;
unsigned int TEX_SQUARE_WIDTH = 4;
unsigned int TEX_SAMPLE_TYPE = GL_NEAREST;

//use these for drawing the more complex figures
float* vertices = NULL;
float* texCoords = NULL;

//torus parameters
unsigned int R = 30;
unsigned int r = 10;
int p = 60;
int q = 60;

float f(int i, int j)
{
  if(mode==2)
    return((R+r*cos((-1+2*(float)j/q)*M_PI))*cos((-1+2*(float)i/p)*M_PI));
  else if(mode==3)
    return (R*sin(j*M_PI/q)*cos(2*i*M_PI/p));
}

float g(int i, int j)
{
  if(mode==2)
    return((R+r*cos((-1+2*(float)j/q)*M_PI))*sin((-1+2*(float)i/p)*M_PI));
  else if(mode==3)
    return (R*sin(j*M_PI/q)*sin(2*i*M_PI/p));
}

float h(int i, int j)
{
  if(mode==2)
    return(r*sin((-1+2*(float)j/q)*M_PI));
  else if(mode==3)
    return (R*cos(j*M_PI/q));
}

void fillVertices(void)
{
  int i,j,k;
  k=0;
  for(j=0; j<=q; ++j)
    for(i=0; i<=p; ++i)
      {
	vertices[k++] = f(i,j);
	vertices[k++] = g(i,j);
	vertices[k++] = h(i,j);
      }
}

void fillTexCoords(void)
{
  int i,j,k;

  k=0;
  for(j=0; j<=q; ++j)
    for(i=0; i<=p; ++i)
      {
	texCoords[k++] = (float)i/p;
	texCoords[k++] = (float)j/q;
      }
}

// Converts our logical game of life array into a valid Texel array for texture mapping
void fillTex()
{
  unsigned int tex_size = TEX_SQUARE_WIDTH*TEX_SQUARE_WIDTH*xSquares*ySquares*3;
  unsigned int tex_width = TEX_SQUARE_WIDTH*xSquares*3;
  unsigned int tex_height = TEX_SQUARE_WIDTH*ySquares;

  if(tex == NULL)
    {
      tex = (unsigned char*)malloc(tex_size);
    }
  else
    {
      free(tex);
      tex = (unsigned char*)malloc(tex_size);
    }
  int i,j,k;
  for(i=0; i<TEX_SQUARE_WIDTH*ySquares; ++i)
    {
      for(j=0; j<TEX_SQUARE_WIDTH*xSquares; ++j)
	{
	  for(k=0; k<3; ++k)
	    {
	      tex[i*tex_width+j*3+k]=255-colors[activeBoard][j/TEX_SQUARE_WIDTH][i/TEX_SQUARE_WIDTH][k];
	    }
	}
    }

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TEX_SAMPLE_TYPE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TEX_SAMPLE_TYPE);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SQUARE_WIDTH*xSquares, TEX_SQUARE_WIDTH*ySquares, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);
}

// Copies a cell from the active board to the inactive board
void copyCell(int cellX, int cellY)
{
  colors[(activeBoard+1)%2][cellX][cellY][0] = colors[activeBoard][cellX][cellY][0];
  colors[(activeBoard+1)%2][cellX][cellY][1] = colors[activeBoard][cellX][cellY][1];
  colors[(activeBoard+1)%2][cellX][cellY][2] = colors[activeBoard][cellX][cellY][2];
}

// Sets a cell on the inactive board to dead (0, 0, 0)
void killCell(int cellX, int cellY)
{
  colors[(activeBoard+1)%2][cellX][cellY][0] = 0;
  colors[(activeBoard+1)%2][cellX][cellY][1] = 0;
  colors[(activeBoard+1)%2][cellX][cellY][2] = 0; 
}

// Check if a cell on the active board is alive (if its color is not white)
bool cellIsAlive(int cellX, int cellY)
{
  return colors[activeBoard][cellX][cellY][0] || colors[activeBoard][cellX][cellY][1] || colors[activeBoard][cellX][cellY][2];
}

// Click calls this to set colors of cells on the active board
void clickActiveCell(int cellX, int cellY)
{
  if(!cellIsAlive(cellX, cellY))
    {
	dragState = 1;
	colors[activeBoard][cellX][cellY][0] = currentRed;
	colors[activeBoard][cellX][cellY][1] = currentGreen;
	colors[activeBoard][cellX][cellY][2] = currentBlue;  
    }
  else
    {
	dragState = 0;
	colors[activeBoard][cellX][cellY][0] = 0;
	colors[activeBoard][cellX][cellY][1] = 0;
	colors[activeBoard][cellX][cellY][2] = 0;  
    }
}

void getWrappedCoordinates(int i, int j, int coordinates[])
{
  coordinates[0] = -1;
  coordinates[1] = -1;
  int tempX = 0;
  int tempY = 0;

  // X wrapping
  if(i < 0)
  {
    if(xWrapMode == 0)
    {
      coordinates[0] = -1;
      coordinates[1] = -1;
      return;
    }
    else
    {
      tempX = xSquares-1;
    }
  }
  else if(i >= xSquares)
  {
    if(xWrapMode == 0)
    {
      coordinates[0] = -1;
      coordinates[1] = -1;
      return;
    }
    else
      tempX = 0;
  }
  else
    tempX = i;

  // Y wrapping
  if(j < 0)
  {
    if(yWrapMode == 0)
    {
      coordinates[0] = -1;
      coordinates[1] = -1;
      return;
    }
    else if (yWrapMode == 2)
    {
      tempY = 0;
      tempX = (tempX+(xSquares/2))%xSquares;
      coordinates[0] = tempX;
      coordinates[1] = tempY;
      return;
    }
    else
      tempY = ySquares-1;
  }
  else if(j >= ySquares)
  {
    if(yWrapMode == 0)
    {
      coordinates[0] = -1;
      coordinates[1] = -1;
      return;
    }
    else if(yWrapMode == 2)
    {
      tempY = ySquares-1;
      tempX = (tempX+(xSquares/2))%xSquares;
      coordinates[0] = tempX;
      coordinates[1] = tempY;
      return;
    }
    else
      tempY = 0;
  }
  else
    tempY = j;

  coordinates[0] = tempX;
  coordinates[1] = tempY;
  return;
}

// Revive Cell
void reviveCell(int cellX, int cellY, int neighbors)
{
  int redTotal = 0;
  int greenTotal = 0;
  int blueTotal = 0;
  int tempX = 0;
  int tempY = 0;
  for(int i = cellX-1; i <= cellX+1; i++)
  {
    for(int j = cellY-1; j <= cellY+1; j++)
	  {
	    // avoid counting self
	    if(i==cellX && j==cellY)
	        continue;

      int coords[2];
      getWrappedCoordinates(i, j, coords);

      if (coords[0] == -1 || coords[1] == -1)
        continue;

      tempX = coords[0];
      tempY = coords[1];

	    if (cellIsAlive(tempX, tempY))
	    {
	      redTotal+=colors[activeBoard][tempX][tempY][0];
	      greenTotal+=colors[activeBoard][tempX][tempY][1];
	      blueTotal+=colors[activeBoard][tempX][tempY][2];
	    }
	  }
  }
  currentRed = redTotal / neighbors;
  currentGreen = greenTotal / neighbors;
  currentBlue = blueTotal / neighbors;
  colors[(activeBoard+1)%2][cellX][cellY][0] = currentRed;
  colors[(activeBoard+1)%2][cellX][cellY][1] = currentGreen;
  colors[(activeBoard+1)%2][cellX][cellY][2] = currentBlue; 
}

// Function to determine how many neighbors are living next to a given cell
int numberOfLivingNeighbors(int cellX, int cellY)
{
  int livingNeighbors = 0;
  int tempX = 0;
  int tempY = 0;
  for(int i = cellX-1; i <= cellX+1; i++)
  {
    for(int j = cellY-1; j <= cellY+1; j++)
	  {
	    // avoid counting self
	    if(i==cellX && j==cellY)
        continue;
      
      int coords[2];
      getWrappedCoordinates(i, j, coords);

      if (coords[0] == -1 || coords[1] == -1)
        continue;

      tempX = coords[0];
      tempY = coords[1];

	    if (cellIsAlive(tempX, tempY))
        	livingNeighbors++;
	  }
  }
  return livingNeighbors;
}

// Propogates life 1 generation
void propogateLife()
{
  for(int i = 0; i < xSquares; i++)
  {
    for(int j = 0; j < ySquares; j++)
	  {
	    int cellNeighbors = numberOfLivingNeighbors(i, j);
	    if(!cellIsAlive(i, j) && cellNeighbors >= minimumRevive && cellNeighbors <= maximumForLife)
	    {
	      reviveCell(i, j, cellNeighbors);
	    }
	    else if(cellIsAlive(i, j) && cellNeighbors >= minimumSurvive && cellNeighbors <= maximumForLife)
	    {
	      copyCell(i, j);
	    }
	    else
	    {
	      killCell(i, j);
	    }
	  }
  }
  activeBoard = (activeBoard+1)%2;
}

// Drawing (display) routine.
void drawScene(void)
{
  int i,j;
  // Clear screen to background color.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set foreground (or drawing) color.
  glColor3f(1.0, 1.0, 1.0);

  if(mode == 0)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // Draw a polygon with specified vertices.

    for(i = 0; i < xSquares; i++)
	  {
	    for(j = 0; j < ySquares; j++)
	    {
	      glColor3ub(255-colors[activeBoard][i][j][0], 255-colors[activeBoard][i][j][1], 255-colors[activeBoard][i][j][2]);
	      glRectf((i*(100/(float)xSquares))+0.2, (j*(100/(float)ySquares))+0.2 , ((i+1)*(100/(float)xSquares))-0.2, ((j+1)*(100/(float)ySquares))-0.2);
	    }
	  }
  }
  else if(mode == 1)
  {
    fillTex();
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_POLYGON);
    glTexCoord2f(0.0, 0.0); glVertex3f(-49.0, -49.0, 0.0);
    glTexCoord2f(1.0, 0.0); glVertex3f(49.0, -49.0, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex3f(49.0, 49.0, 0.0);
    glTexCoord2f(0.0, 1.0); glVertex3f(-49, 49.0, 0.0);
    glEnd();
  }
  else if(mode >= 2)
    {
      fillTex();
      glBindTexture(GL_TEXTURE_2D, texture);
      vertices = new float[3*(p+1)*(q+1)];
      texCoords = new float[2*(p+1)*(q+1)];

      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
      
      fillVertices();
      fillTexCoords();

      for(j=0; j<q; ++j)
	{
	  glBegin(GL_TRIANGLE_STRIP);
	  for(i=0; i<=p; ++i)
	    {
	      glArrayElement((j+1)*(p+1)+i);
	      glArrayElement(j*(p+1)+i);
	    }
	  glEnd();
	}
      free(vertices);
      free(texCoords);
    }
  glutSwapBuffers();   
}

// Initialization routine.
void setup(void) 
{
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  // Set background (or clearing) color.
  glClearColor(0.0, 0.0, 0.0, 0.0); 
  glGenTextures(1,&texture);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
  // Set viewport size to be entire OpenGL window.
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  windowWidth = w;
  windowHeight = h;
  
  // Set matrix mode to projection.
  glMatrixMode(GL_PROJECTION);

  // Clear current projection matrix to identity.
  glLoadIdentity();

  // Specify the orthographic (or perpendicular) projection, 
  // i.e., define the viewing box.
  glOrtho(0.0, 100.0, 0.0, 100.0, -40.0, 140.0);

  // Set matrix mode to modelview.
  glMatrixMode(GL_MODELVIEW);

  // Clear current modelview matrix to identity.
  glLoadIdentity();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
  switch(key) 
  {
      // Press escape to exit.
    case 27:
      exit(0);
      break;
    default:
      break;

  case 'x':
    if (xSquares < X_MAX)
    {
	    xSquares+=2;
	    drawScene();
    }
    break;

  case 'X':
    if (xSquares > 3)
    {
	    xSquares-=2;
	    drawScene();
    }
    break;

  case 'y':
    if (ySquares < Y_MAX)
    {
	    ySquares++;
	    drawScene();
    }
    break;

  case 'Y':
    if (ySquares > 3)
    {
	    ySquares--;
	    drawScene();
    }
    break;

  case 's':
    stepPeriod = stepPeriod*0.9;
    if (stepPeriod < minStepPeriod)
	  {
	    stepPeriod = minStepPeriod;
	  }
    break;

  case 'S':
    stepPeriod = stepPeriod*1.1;
    if (stepPeriod > maxStepPeriod)
	  {
	    stepPeriod = maxStepPeriod;
	  }
    break;

  case 'n':
    propogateLife();
    glutPostRedisplay();
    break;

  case 'R':
    TEX_SQUARE_WIDTH = TEX_SQUARE_WIDTH >> 1;
    if(!TEX_SQUARE_WIDTH)
      TEX_SQUARE_WIDTH = 1u;
    break;

  case 'r':
    TEX_SQUARE_WIDTH = TEX_SQUARE_WIDTH << 1;
    if(!TEX_SQUARE_WIDTH)
      TEX_SQUARE_WIDTH = 1u << sizeof(TEX_SQUARE_WIDTH);
    break;

  case 't':
    if(TEX_SAMPLE_TYPE == GL_LINEAR)
      TEX_SAMPLE_TYPE=GL_NEAREST;
    else
      TEX_SAMPLE_TYPE=GL_LINEAR;
    break;

  case 'p':
    p += 3;
    if(p<0)
      p=INT_MAX;
    break;
  case 'P':
    p -= 3;
    if(p<=0)
      p=3;
    break;

  case 'q':
    q += 3;
    if(q<0)
      q=INT_MAX;
    break;
  case 'Q':
    q -= 3;
    if(q<=0)
      q=3;
    break;

  case ' ':
    isStepping = !isStepping;
    break;

  case ',':
    if(mode)
      {
	glRotatef(2.0,1.0,0.0,0.0);
	drawScene();
      }
    break;
  case '.':
    if(mode)
      {
	glRotatef(2.0,0.0,1.0,0.0);
	drawScene();
      }
    break;
  case '/':
    if(mode)
      {
	glRotatef(2.0,0.0,0.0,1.0);
	drawScene();
      }
    break;

 case '<':
   if(mode)
     {
       glRotatef(-2.0,1.0,0.0,0.0);
       drawScene();
     }
    break;
  case '>':
    if(mode)
      {
	glRotatef(-2.0,0.0,1.0,0.0);
	drawScene();
      }
    break;
  case '?':
    if(mode)
      {
	glRotatef(-2.0,0.0,0.0,1.0);
	drawScene();
      }
    break;

  case '3':
    mode=3;
    xWrapMode=2;
    yWrapMode=2;
    glLoadIdentity();
    glTranslatef(50.0,50.0,-50.0);
    drawScene();
    break;
  case '2':
    mode = 2;
    xWrapMode = 1;
    yWrapMode = 1;
    glLoadIdentity();
    glTranslatef(50.0,50.0,-50.0);
    drawScene();
    break;
  case '1':
    mode = 1;
    xWrapMode = 0;
    yWrapMode = 0;
    glLoadIdentity();
    glTranslatef(50.0,50.0,-50.0);
    drawScene();
    break;
  case '0':
    mode = 0;
    xWrapMode = 0;
    yWrapMode = 0;
    glLoadIdentity();
    drawScene();
    break;
    }
}

// Mouse functions for clicking cells to toggle alive/dead
void mouseControl(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
      int cellX = floor(x/(windowWidth/(float)xSquares));
      int cellY = ySquares - 1 - floor(y/(windowHeight/(float)ySquares));
      currentRed = cursorRed;
      currentGreen = cursorGreen;
      currentBlue = cursorBlue;
      clickActiveCell(cellX, cellY);
    }
  glutPostRedisplay();
}

void mouseMotion(int x, int y)
{     
  int cellX = floor(x/(windowWidth/(float)xSquares));
  int cellY = ySquares - 1 - floor(y/(windowHeight/(float)ySquares));
  if(0 <= cellX && cellX < xSquares && 0 <= cellY && cellY < ySquares){
  	if(dragState == 1){
      		colors[activeBoard][cellX][cellY][0] = cursorRed;
     	 	colors[activeBoard][cellX][cellY][1] = cursorGreen;
      		colors[activeBoard][cellX][cellY][2] = cursorBlue;  
    	}
  	else{
      		colors[activeBoard][cellX][cellY][0] = 0;
      		colors[activeBoard][cellX][cellY][1] = 0;
      		colors[activeBoard][cellX][cellY][2] = 0;  
    	}
  	glutPostRedisplay();
  }
}

// One step of life
void step(int value)
{
  if (isStepping) 
    {
      propogateLife();
    }
  glutTimerFunc(stepPeriod, step, 1);
  glutPostRedisplay();
}

// Menu of colors available
void colorMenu(int id)
{
  if (id==1) 
  {
    cursorRed = 0;
    cursorGreen = 255;
    cursorBlue = 255;
  }
  if (id==2)
  {
    cursorRed = 0;
    cursorGreen = 127;
    cursorBlue = 255;
  }
  if (id==3)
  {
    cursorRed = 0;
    cursorGreen = 0;
    cursorBlue = 255;
  }
  if (id==4)
  {
    cursorRed = 255;
    cursorGreen = 0;
    cursorBlue = 255;
  }
  if (id==5)
  {
    cursorRed = 255;
    cursorGreen = 255;
    cursorBlue = 0;
  }
  if (id==6)
  {
    cursorRed = 180;
    cursorGreen = 255;
    cursorBlue = 125;
  }
  if (id==7)
  {
    cursorRed = 255;
    cursorGreen = 255;
    cursorBlue = 255;
  }
}

void wrappingMenu(int id)
{
  if (id==8)
  {
    xWrapMode = 0;
    yWrapMode = 0;
  }
  if (id==9)
  {
    xWrapMode = 1;
    yWrapMode = 1;
  }
  if (id==10)
  {
    xWrapMode = 1;
    yWrapMode = 2;
  }
}

// Menu for adding colors
void mainMenu(int id)
{
  if (id == 11) 
    exit(0);
}

void makeMenu(void)
{
  int sub_menu_colors;
  int sub_menu_wrapping;
  sub_menu_colors = glutCreateMenu(colorMenu);
  glutAddMenuEntry("Red", 1);
  glutAddMenuEntry("Orange",2);
  glutAddMenuEntry("Yellow", 3);
  glutAddMenuEntry("Green", 4);
  glutAddMenuEntry("Blue", 5);
  glutAddMenuEntry("Purple", 6);
  glutAddMenuEntry("Black", 7);

  sub_menu_wrapping = glutCreateMenu(wrappingMenu);
  glutAddMenuEntry("No Wrapping", 8);
  glutAddMenuEntry("Full Wrapping", 9);
  glutAddMenuEntry("Sphere Wrapping", 10);

  glutCreateMenu(mainMenu);
  glutAddSubMenu("Colors", sub_menu_colors);
  glutAddSubMenu("Wrapping", sub_menu_wrapping);
  glutAddMenuEntry("Quit",11);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}


// Main routine: defines window properties, creates window,
// registers callback routines and begins processing.
int main(int argc, char **argv) 
{  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100); 
  glutCreateWindow("Game of Life");
  // Initializing
  setup(); 
  glutDisplayFunc(drawScene); 
  glutReshapeFunc(resize);  
  glutKeyboardFunc(keyInput);
  glutMouseFunc(mouseControl); 
  glutMotionFunc(mouseMotion);
  glutTimerFunc(5, step, 1);
  makeMenu();
  glutMainLoop(); 

  return 0;  
}
