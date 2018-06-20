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
#ifdef __OSX__
# include <glx.h>
# include <glu.h>
# include <glut.h>
#else
# include <GL/glx.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include <math.h>
#include <cv.h>
#include <highgui.h>

using std::vector;

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
    KeySym                  key;
    char                    text[1];

    double rho, theta, phi, alpha, speed, X, Y, Z;
    double IK, JL, AD, WS, TG, FH, UO, QE, RY;

    Gdisplay(int,const char*, double, double, double);
    void setTitle(char*);
    void resetDisplay(vector <double>, vector <double>, vector <double>);
    void redrawDisplay();
    void closeDisplay();
    void drawHex(double,double,double,double,double,double,double);
    void drawHexSeg(double x,double y,double z,double r,double red,double green,double blue,int q);
    // void drawTri(vector <double>,vector <double>,vector <double>,double,double,double);
    void drawTri(vector <double>,vector <double>,vector <double>,vector <double>);
    void drawTriFill(vector <double>,vector <double>,vector <double>,vector <double>);
    void drawSphere(double,double,double,double,vector <double>,int);
    void drawLine(double,double,double,double,double,double,double,double,double,double);
    void addCrossHairs(double,double,int);
    void addFloor(double,double);
    // void drawCylinder(vector <double>,vector <double>,double, double, double, vector <double>,int);
    void addQuad(vector <double>,vector <double>,vector <double>, vector <double>,vector <double>, vector <double>);

    void saveImage(std::string);
    void drawCylinder(float, float, float, float, float, float, float, float,int,vector <double>);

    void drawMesh(vector< vector< vector <double> > >, vector<vector<vector<double> > >);
    void drawTorus(vector< vector< vector <double> > >, vector<vector<vector<double> > >);
    void drawMesh2(vector< vector< vector <double> > >, vector <double>);

    void drawCubeSphere(vector< vector <double> > );
    // void drawFlatCube(vector< vector <double> > );
    void drawSphereFromMesh(vector<vector<double> >, vector<vector<int> >,vector<vector<double> >);
    void drawFlatCube(vector< vector<int> >, vector< vector<double> >,double,double,double);
};

#endif /* defined(____display__) */
