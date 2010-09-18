#!/usr/bin/env python
# ===========================================================================

from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *
import sys

cubeTranslationX = 0
cubeTranslationY = 0
cubeRotation = 0

def drawScene():
    global cubeTranslationX, cubeTranslationY
    global cubeRotation

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)    

    # Draw Text
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -7.0)
    glColor3f(1.0, 0.0, 0.0)
    glRasterPos2f(-4.6, 2.3)
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, 'OpenGL 3D Cube')

    # Setup Cube Rotation/Translation
    glLoadIdentity()
    glTranslatef(cubeTranslationX, cubeTranslationY, -7.0)
    glRotatef(cubeRotation, 1.0, 1.0, 1.0)

    # Draw Cube
    glBegin(GL_QUADS)

    glColor3f(0.0, 1.0, 0.0)
    glVertex3f( 1.0,  1.0, -1.0)
    glVertex3f(-1.0,  1.0, -1.0)
    glVertex3f(-1.0,  1.0,  1.0)
    glVertex3f( 1.0,  1.0,  1.0)

    glColor3f(1.0, 0.5, 0.0)
    glVertex3f( 1.0, -1.0,  1.0)
    glVertex3f(-1.0, -1.0,  1.0)
    glVertex3f(-1.0, -1.0, -1.0)
    glVertex3f( 1.0, -1.0, -1.0)

    glColor3f(1.0, 0.0, 0.0)
    glVertex3f( 1.0,  1.0,  1.0)
    glVertex3f(-1.0,  1.0,  1.0)
    glVertex3f(-1.0, -1.0,  1.0)
    glVertex3f( 1.0, -1.0,  1.0)

    glColor3f(1.0, 1.0, 0.0)
    glVertex3f( 1.0, -1.0, -1.0)
    glVertex3f(-1.0, -1.0, -1.0)
    glVertex3f(-1.0,  1.0, -1.0)
    glVertex3f( 1.0,  1.0, -1.0)

    glColor3f(0.0, 0.0, 1.0)
    glVertex3f(-1.0,  1.0,  1.0)
    glVertex3f(-1.0,  1.0, -1.0)
    glVertex3f(-1.0, -1.0, -1.0)
    glVertex3f(-1.0, -1.0,  1.0)

    glColor3f(1.0, 0.0, 1.0)
    glVertex3f( 1.0,  1.0, -1.0)
    glVertex3f( 1.0,  1.0,  1.0)
    glVertex3f( 1.0, -1.0,  1.0)
    glVertex3f( 1.0, -1.0, -1.0)

    glEnd()

    cubeRotation += 0.20

    glutSwapBuffers()

def resizeScene(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, float(width) / height, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)

def keyPressed(*args):
    if (args[0] == '\033'):
        sys.exit(0)

def specialKeyPressed(*args):
    global cubeTranslationX, cubeTranslationY

    # Update Cube Translation X, Y using Keyboard
    if (args[0] == GLUT_KEY_LEFT):
        cubeTranslationX -= 0.1
    elif (args[0] == GLUT_KEY_RIGHT):
        cubeTranslationX += 0.1
    elif (args[0] == GLUT_KEY_UP):
        cubeTranslationY += 0.1
    elif (args[0] == GLUT_KEY_DOWN):
        cubeTranslationY -= 0.1

    # Fix the Translation X Value
    if (cubeTranslationX < -7.1):
        cubeTranslationX = 7.1
    if (cubeTranslationX > 7.1):
        cubeTranslationX = -7.1

    # Fix the Translation Y Value
    if (cubeTranslationY > 4.3):
        cubeTranslationY = -4.3
    elif (cubeTranslationY < -4.3):
        cubeTranslationY = 4.3

def initGl(width, height):
    glClearColor(0.0, 0.0, 0.0, 0.0)

    glClearDepth(1.0)
    glDepthFunc(GL_LESS)
    glEnable(GL_DEPTH_TEST)

    glShadeModel(GL_SMOOTH)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45.0, float(width) / height, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)

def main():
    glutInit(sys.argv)

    winWidth = 540
    winHeight = 320

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH)

    glutInitWindowSize(winWidth, winHeight)
    window = glutCreateWindow('OpenGL 3D Cube')

    # Custom Handlers
    glutSpecialFunc(specialKeyPressed)
    glutKeyboardFunc(keyPressed)
    glutReshapeFunc(resizeScene)
    glutDisplayFunc(drawScene)
    glutIdleFunc(drawScene)

    initGl(winWidth, winHeight)

    glutMainLoop()

if (__name__ == '__main__'):
    main()

