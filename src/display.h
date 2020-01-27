/*!
 * display.h
 *
 * Graphics code.
 *
 * Created by Stuart P Wilson on 24/12/2012.
 */

#ifndef ____display__
#define ____display__

#include <X11/X.h>
#include <X11/Xlib.h>
# include <GL/glx.h>
# include <GL/glu.h>
#ifdef __OSX__
# include <GLUT/glut.h>
#else
# include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include <array>
#include <math.h>

using std::vector;
using std::array;

namespace morph {

    /*!
     * A class for drawing objects on an OpenGL screen
     */
    class Gdisplay
    {
    public:
        Display                 *disp;
        Window                  root;
        XVisualInfo             *vi;
        Colormap                cmap;
        XSetWindowAttributes    swa;
        Window                  win;
        XClassHint*             classHints;
        XWMHints*               wmHints;
        GLXContext              glc;
        XWindowAttributes       gwa;
        XEvent                  xev;
        KeySym                  key;
        char                    text[1];

        double rho, theta, phi, alpha, speed, X, Y, Z;
        double IK, JL, AD, WS, TG, FH, UO, QE, RY;

    private:

        /*!
         * The aspect ratio of the window, taken from the windowWidth
         * and windowHeight used in the construction of the Gdisplay.
         */
        GLfloat x_aspect_ratio;

        /*!
         * Common to all constructors. Create an Xwindow and GL context
         */
        void createWindow (unsigned int windowWidth, unsigned int windowHeight,
                           unsigned int x, unsigned int y,
                           const char* title, XID firstWindow = (XID)0x0);

    public:

        /*!
         * Constructor for square displays
         */
        Gdisplay(int,const char*, double, double, double);

        /*!
         * Constructor for OpenGL displays (i.e. windows) that are
         * rectangular.
         *
         * If you want to group your windows (so that the window
         * manager knows that they're all part of the same program),
         * then pass in the win attribute of the first window as the
         * firstWindow argument here. For example:
         *
         * Gdisplay d1(1020, 300, winTitle.c_str(), rhoInit, 0.0, 0.0);
         * Gdisplay d2(1020, 300, winTitle.c_str(), rhoInit, 0.0, 0.0, d1.win);
         *
         * @param windowWidth The width of the window in pixels
         *
         * @param windowHeight The height of the window in pixels
         *
         * @param x horizontal position of window
         *
         * @param y vertical position of window
         *
         * @param title The window title to show in the window bar
         *
         * @param rhoInit Part of spherical coordinates for the
         * initial view into the GLX context
         *
         * @param thetaInit Part of spherical coordinates for the
         * initial view into the GLX context
         *
         * @param phiInit Part of spherical coordinates for the
         * initial view into the GLX context
         *
         * @param firstWindow The XID of the first window in the group
         * of windows that this Gdisplay should be a member of.
         */
        Gdisplay (unsigned int windowWidth, unsigned int windowHeight,
                  unsigned int x, unsigned int y,
                  const char* title,
                  double rhoInit, double thetaInit, double phiInit,
                  XID firstWindow = (XID)0x0);

        void setTitle(char*);
        void resetDisplay(vector <double>, vector <double>, vector <double>);
        void redrawDisplay();
        void closeDisplay();
        void drawHex(double,double,double,double,double,double,double);
        /*!
         * A drawHex taking float args. pos is the position of the
         * hex. r is the shortest distance from the centre of the Hex
         * to the perimeter. c is the colour to draw the hex with.
         */
        void drawHex (array<float, 3> pos, float r, array<float, 3> c);
        /*!
         * A drawHex taking float args and an offset. pos is the
         * position of the hex. r is the shortest distance from the
         * centre of the Hex to the perimeter. c is the colour to draw
         * the hex with.
         */
        void drawHex (array<float, 3> pos, array<float, 3> offset, float r, array<float, 3> c);

        /*!
         * Use only x and y from pos to determine position in 2D of
         * hex, set z position to val, to give a 3D coloured graph
         * (determining colour from val)
         */
        void drawHex (array<float,3> pos, array<float,3> offset, float r, float val);

        /*!
         * SW: Draw single line segment of the hex, with edgeIndex in 0 to 5 indexing anti-clockwise from East.
         */
        void drawHexSeg(array<float,3> pos,array<float,3> offset,double r,array<float,3> col,int q);

        /*!
         * Draw a triangle.
         */
        void drawTri (vector <double> p1, vector <double> p2, vector <double> p3, vector <double> C);

        /*!
        * Draw a rectangle
        */
        void drawRect(double x, double y, double z, double width, double height, vector<double> color);

        /*!
         * Draw a filled triangle of colour cl with vertices at points
         * p1, p2 and p3. p1-p3 are vectors in three-space. C is a
         * three component RGB colour specification.
         */
        //@{
        void drawTriFill (vector <double> p1, vector <double> p2, vector <double> p3, vector <double> C);
        void drawTriFill (array<float, 3> p1, array<float, 3> p2, array<float, 3> p3, array<float, 3> C);
        //@}

        void drawSphere(double,double,double,double,vector <double>,int);
        void drawLine(double,double,double,double,double,double,double,double,double,double);
        void addCrossHairs(double,double,int);
        void addFloor(double,double);
        void addQuad(vector <double>,vector <double>,vector <double>, vector <double>,vector <double>, vector <double>);

        void saveImage(std::string);
        void drawCylinder(float, float, float, float, float, float, float, float,int,vector <double>);

        void drawMesh(vector< vector< vector <double> > >, vector<vector<vector<double> > >);
        void drawTorus(vector< vector< vector <double> > >, vector<vector<vector<double> > >);
        void drawMesh2(vector< vector< vector <double> > >, vector <double>);

        void drawCubeSphere(vector< vector <double> > );
        void drawSphereFromMesh(vector<vector<double> >, vector<vector<int> >,vector<vector<double> >);
        void drawFlatCube(vector< vector<int> >, vector< vector<double> >,double,double,double);
    };

} // namespace morph

#endif /* defined(____display__) */
