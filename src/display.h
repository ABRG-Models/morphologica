//
//  display.h
//
//
//  Created by Stuart P Wilson on 24/12/2012.
//
//

#ifndef ____display__
#define ____display__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <math.h>
#include "cv.h"
#include "highgui.h"

class Gdisplay
{

private:

    XSetWindowAttributes attributes;

public:

    Display                 *disp;
    Window                  root;
    XVisualInfo             *vi;
    Colormap                cmap;
    XSetWindowAttributes    swa;
    Window                  win;
    GLXContext              glc;
    XWindowAttributes       gwa;
    XEvent                  xev;
    KeySym					key;
    char					text[1];

    double rho, theta, phi, alpha, speed, X, Y, Z;
    double IK, JL, AD, WS, TG, FH, UO, QE, RY;

    Gdisplay(int,const char*, double, double, double);
    void setTitle(char*);
    void resetDisplay(std::vector <double>, std::vector <double>, std::vector <double>);
    void redrawDisplay();
    void closeDisplay();
    void drawHex(double,double,double,double,double,double,double);
    void drawHexSeg(double x,double y,double z,double r,double red,double green,double blue,int q);
    // void drawTri(std::vector <double>,std::vector <double>,std::vector <double>,double,double,double);
    void drawTri(std::vector <double>,std::vector <double>,std::vector <double>,std::vector <double>);
    void drawTriFill(std::vector <double>,std::vector <double>,std::vector <double>,std::vector <double>);
    void drawSphere(double,double,double,double,std::vector <double>,int);
    void drawLine(double,double,double,double,double,double,double,double,double,double);
    void addCrossHairs(double,double,int);
    void addFloor(double,double);
    // void drawCylinder(std::vector <double>,std::vector <double>,double, double, double, std::vector <double>,int);
    void addQuad(std::vector <double>,std::vector <double>,std::vector <double>, std::vector <double>,std::vector <double>, std::vector <double>);

    void saveImage(std::string);
    void drawCylinder(float, float, float, float, float, float, float, float,int,std::vector <double>);

    void drawMesh(std::vector< std::vector< std::vector <double> > >, std::vector<std::vector<std::vector<double> > >);
    void drawTorus(std::vector< std::vector< std::vector <double> > >, std::vector<std::vector<std::vector<double> > >);
    void drawMesh2(std::vector< std::vector< std::vector <double> > >, std::vector <double>);

    void drawCubeSphere(std::vector< std::vector <double> > );
    // void drawFlatCube(std::vector< std::vector <double> > );
    void drawSphereFromMesh(std::vector<std::vector<double> >, std::vector<std::vector<int> >,std::vector<std::vector<double> >);
    void drawFlatCube(std::vector< std::vector<int> >, std::vector< std::vector<double> >,double,double,double);
};

#endif /* defined(____display__) */
