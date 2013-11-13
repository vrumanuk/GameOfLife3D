////////////////////////////////////////////////////
// square.cpp
//
// Stripped down OpenGL program that draws a square.
//
// Sumanta Guha.
////////////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include <cmath>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

const int X_MAX = 100;
const int Y_MAX = 100;

bool isStepping = 0;
unsigned char activeBoard = 0;
unsigned char mode = 0;
// ADD A WAY FOR USERS TO CHANGE THIS
unsigned int stepPeriod = 400;
unsigned int maxStepPeriod = 5000;
unsigned int minStepPeriod = 50;

// ADD A MENU FOR USERS TO CHANGE THIS
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
unsigned int xSquares = 10;
unsigned int ySquares = 10;
unsigned int TEX_SQUARE_WIDTH = 4;
void fillTex()
{
  unsigned int tex_size = TEX_SQUARE_WIDTH*TEX_SQUARE_WIDTH*xSquares*ySquares*3;
  unsigned int tex_width = TEX_SQUARE_WIDTH*xSquares*3;
  unsigned int tex_height = TEX_SQUARE_WIDTH*ySquares;

  if(tex == NULL)
    {
      tex = (unsigned char*)malloc(tex_size);
    }
  else if(sizeof(tex) != tex_size)
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
	      tex[i*tex_width+j*3+k]=colors[activeBoard][i/TEX_SQUARE_WIDTH][j/TEX_SQUARE_WIDTH][k];
	    }
	}
    }

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  //Not sure what's up with this line. If it's uncommented it causes
  //a black screen you can't get back from if you ever set mode=1.
  
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64/*TEX_SQUARE_WIDTH*xSquares*/, 64/*TEX_SQUARE_WIDTH*ySquares*/, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);
}

void copyCell(int cellX, int cellY)
{
   colors[(activeBoard+1)%2][cellX][cellY][0] = colors[activeBoard][cellX][cellY][0];
   colors[(activeBoard+1)%2][cellX][cellY][1] = colors[activeBoard][cellX][cellY][1];
   colors[(activeBoard+1)%2][cellX][cellY][2] = colors[activeBoard][cellX][cellY][2];
}

void killCell(int cellX, int cellY)
{
   colors[(activeBoard+1)%2][cellX][cellY][0] = 0;
   colors[(activeBoard+1)%2][cellX][cellY][1] = 0;
   colors[(activeBoard+1)%2][cellX][cellY][2] = 0; 
}

bool cellIsAlive(int cellX, int cellY)
{
   return colors[activeBoard][cellX][cellY][0] || colors[activeBoard][cellX][cellY][1] || colors[activeBoard][cellX][cellY][0];
}

void clickActiveCell(int cellX, int cellY)
{
   if(!cellIsAlive(cellX, cellY))
   {
      colors[activeBoard][cellX][cellY][0] = currentRed;
      colors[activeBoard][cellX][cellY][1] = currentGreen;
      colors[activeBoard][cellX][cellY][2] = currentBlue;  
   }
   else
   {
      colors[activeBoard][cellX][cellY][0] = 0;
      colors[activeBoard][cellX][cellY][1] = 0;
      colors[activeBoard][cellX][cellY][2] = 0;  
   }
}

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

         // X wrapping
         if(i >= xSquares)
            tempX = 0;
         else if(i < 0)
            tempX = xSquares-1;
         else
            tempX = i;

         // Y wrapping
         if(j >= ySquares)
            tempY = 0;
         else if(j < 0)
            tempY = ySquares-1;
         else
            tempY = j;
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

         // X wrapping
         if(i >= xSquares)
            tempX = 0;
         else if(i < 0)
            tempX = xSquares-1;
         else
            tempX = i;

         // Y wrapping
         if(j >= ySquares)
            tempY = 0;
         else if(j < 0)
            tempY = ySquares-1;
         else
            tempY = j;

         if (cellIsAlive(tempX, tempY))
            livingNeighbors++;
      }
   }
   return livingNeighbors;
}

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
   // Clear screen to background color.
   glClear(GL_COLOR_BUFFER_BIT);

   // Set foreground (or drawing) color.
   glColor3f(1.0, 1.0, 1.0);

   if(mode == 0)
     {
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
       // Draw a polygon with specified vertices.
       
       for(int i = 0; i < xSquares; i++)
	 {
	   for(int j = 0; j < ySquares; j++)
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
     glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, -10.0, 0.0);
     glTexCoord2f(1.0, 0.0); glVertex3f(10.0, -10.0, 0.0);
     glTexCoord2f(1.0, 1.0); glVertex3f(10.0, 10.0, 0.0);
     glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, 10.0, 0.0);
     glEnd();
   }
   glutSwapBuffers();   
}

// Initialization routine.
void setup(void) 
{
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
   glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);

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
         if (xSquares < X_MAX){
            xSquares++;
            drawScene();
         }
      break;

      case 'X':
         if (xSquares > 3){
            xSquares--;
            drawScene();
         }
      break;

      case 'y':
         if (ySquares < Y_MAX){
            ySquares++;
            drawScene();
         }
      break;

      case 'Y':
         if (ySquares > 3){
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

   case 'p':
         propogateLife();
         glutPostRedisplay();
      break;

   case ' ':
         isStepping = !isStepping;
      break;
      
   case '1':
     mode = 1;
     drawScene();
     break;
   case '0':
     mode = 0;
     drawScene();
     break;

   }
}


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

void step(int value)
{
   if (isStepping) 
   {
      propogateLife();
   }
   glutTimerFunc(stepPeriod, step, 1);
   glutPostRedisplay();
}

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
      cursorRed = 255;
      cursorGreen = 0;
      cursorBlue = 255;
   }
   if (id==3)
   {
      cursorRed = 255;
      cursorGreen = 255;
      cursorBlue = 0;
   }
}

void mainMenu(int id)
{
   if (id==4) 
      exit(0);
}

void makeMenu(void)
{
   int sub_menu;
   sub_menu = glutCreateMenu(colorMenu);
   glutAddMenuEntry("Red", 1);
   glutAddMenuEntry("Green",2);
   glutAddMenuEntry("Blue", 3);

   glutCreateMenu(mainMenu);
   glutAddSubMenu("Colors", sub_menu);
   glutAddMenuEntry("Quit",4);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
}


// Main routine: defines window properties, creates window,
// registers callback routines and begins processing.
int main(int argc, char **argv) 
{  
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); 
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(100, 100); 
   glutCreateWindow("Game of Life");
   // Initializing
   setup(); 
   glutDisplayFunc(drawScene); 
   glutReshapeFunc(resize);  
   glutKeyboardFunc(keyInput);
   glutMouseFunc(mouseControl); 
   glutTimerFunc(5, step, 1);
   makeMenu();
   glutMainLoop(); 

   return 0;  
}
