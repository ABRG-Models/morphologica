#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/imgcodecs.hpp>
#include "morph/display.h"
#ifdef __ICC__
# define ARMA_ALLOW_FAKE_GCC 1
#endif
#include <armadillo>
#include <stdexcept>

using std::runtime_error;

morph::Gdisplay::Gdisplay (int myWindowSize, const char* title, double rhoInit, double thetaInit, double phiInit)
{
    this->createWindow ((unsigned int)myWindowSize, (unsigned int)myWindowSize, 0, 0, title);

    this->speed = 5.*acos(-1.)/180.; // in degrees
    this->rho = 2.5 + rhoInit;
    this->theta = (thetaInit + 0.5)*acos(-1.);
    this->phi   = (phiInit + 0.00000001)*acos(-1.);
    alpha = 0.;
    Z = 1.;
}

// A more flexible constructor.
morph::Gdisplay::Gdisplay (unsigned int windowWidth, unsigned int windowHeight,
                           unsigned int x, unsigned int y,
                           const char* title,
                           double rhoInit, double thetaInit, double phiInit, XID firstWindow)
{
    this->createWindow (windowWidth, windowHeight, x, y, title, firstWindow);

    this->speed = 5.*acos(-1.)/180.; // in degrees
    this->rho = rhoInit; // Stuart did have rhoInit+2.5, for some reason. I prefer to have client code pass in the initial rho with no modification made.
    this->theta = (thetaInit + 0.5)*acos(-1.); // Why this? acos(-1) is pi. so , theta is rotated by pi/2
    this->phi   = (phiInit + 0.00000001)*acos(-1.); // Phi is not rotated.
    alpha = 0.;
    Z = 1.;
}

void
morph::Gdisplay::createWindow (unsigned int windowWidth, unsigned int windowHeight,
                               unsigned int x, unsigned int y, const char* title, XID firstWindow)
{
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    this->disp = XOpenDisplay (NULL);
    if (this->disp == NULL) {
        printf ("\n\tcannot connect to X server\n\n");
        throw runtime_error ("Gdisplay: Cannot connect to X server");
    }
    this->root = DefaultRootWindow (this->disp);
    this->vi = glXChooseVisual (this->disp, 0, att);
    if (this->vi == NULL) {
        printf ("\n\tno appropriate visual found\n\n");
        throw runtime_error ("Gdisplay: No appropriate visual found");
    } else {
        printf ("\n\tvisual %p selected\n", (void *)vi->visualid);
    }
    this->x_aspect_ratio = (GLfloat)windowWidth / (GLfloat)windowHeight;
    this->cmap = XCreateColormap (this->disp, this->root, this->vi->visual, AllocNone);
    this->swa.colormap = this->cmap;
    this->swa.event_mask = ExposureMask | KeyPressMask;
    this->win = XCreateWindow (this->disp, this->root, x, y, windowWidth, windowHeight, 0,
                               this->vi->depth, InputOutput, this->vi->visual, CWColormap | CWEventMask, &this->swa);

    glc = glXCreateContext (this->disp, vi, NULL, GL_TRUE);
    XMapWindow (this->disp, this->win);
    XStoreName (this->disp, this->win, (char*)title);

    // Setting the class hint, as well as the window title ensures
    // Gnome 3 (and presumably some other window managers) will
    // display the window name in the icon list.
    // Set the class name for window managers to match morphologica windows and
    // apply layout rules.
    this->classHints = XAllocClassHint();
    this->classHints->res_class = (char*)"morphologica";
    this->classHints->res_name = (char*)title;
    XSetClassHint (this->disp, this->win, this->classHints);

    // Similarly for Window Manager Hints. Using the group here, but
    // could also set an icon.
    if (firstWindow != (XID)0x0) {
        this->wmHints = XAllocWMHints();
        this->wmHints->flags = WindowGroupHint;
        this->wmHints->window_group = firstWindow;
        XSetWMHints (this->disp, this->win, this->wmHints);
    }
}

void
morph::Gdisplay::setTitle (char* title)
{
    XStoreName (disp, win, (char*) title);
}

void
morph::Gdisplay::closeDisplay (void)
{
    glXDestroyContext(disp, glc);
    XDestroyWindow(disp, win);
    XCloseDisplay(disp);
}

void
morph::Gdisplay::resetDisplay (std::vector<double> fix, std::vector<double> eye, std::vector<double> rot)
{
    glXMakeCurrent(disp, win, glc);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0,  // "Specifies the field of view angle, in degrees, in the y direction."
                   this->x_aspect_ratio,   // "Aspect ratio of x to y"
                   0.1,   // "Specifies the distance from the viewer to the near clipping plane (always positive)."
                   20.0); // "Specifies the distance from the viewer to the far clipping plane (always positive)."

    glClearColor(1.0, 1.0, 1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    IK =0.;JL=0.;WS=0.;AD=0.;TG=0.;FH=0.;UO=0.;QE=0.;RY=0.;

    // event processing
    if(XCheckWindowEvent(disp, win, KeyPressMask, &xev)){

        if (XLookupString(&xev.xkey,text,1,&key,0)==1){
            switch(text[0]){

                // primary keyboard controls
            case 'i': IK = +1.;   break; //
            case 'k': IK = -1.;   break; //
            case 'j': JL = -1.;   break; //
            case 'l': JL = +1.;   break; //
            case 'u': UO = -1.;   break; //
            case 'o': UO = +1.;   break; //

                // secondary keyboard controls
            case 'w': WS = +1.;   break; //
            case 's': WS = -1.;   break; //
            case 'a': AD = -1.;   break; //
            case 'd': AD = +1.;   break; //
            case 'q': QE = -1.;   break; //
            case 'e': QE = +1.;   break; //

                // tertiary keyboard controls
            case 't': TG = +1.;   break; //
            case 'g': TG = -1.;   break; //
            case 'f': FH = -1.;   break; //
            case 'h': FH = +1.;   break; //
            case 'r': RY = -1.;   break; //
            case 'y': RY = +1.;   break; //

            }
        }

    }

    rho     = fmax(rho-WS*speed,0.);
    phi     += IK*speed;
    theta   += JL*speed;

    // spherical coordinates (rho=distance,theta=azimuth, i.e., horizontal,phi=polar)
    GLfloat eeyex = rho*sin(phi)*cos(theta);
    GLfloat eeyey = rho*sin(phi)*sin(theta);
    GLfloat eeyez = eye[2]+rho*cos(phi);
    gluLookAt(eeyex, // eyex (co-ordinate of camera)
              eeyey, // eyey
              eeyez, // eyez
              0.,    // centerx (eye fixation point, i.e., 0,0,0)
              0.,    // centery
              0.,    // centerz
              0.,    // upx     (which way is up)
              0.,    // upy
              -sin(phi)); // upz

    glEnable(GL_DEPTH_TEST);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glEnable(GL_LIGHTING);

    GLfloat qaAmbientLight[] = {.2,.2,.2,1.};
    GLfloat qaDiffuseLight[] = {.8,.8,.8,1.};
    GLfloat qaSpecularLight[] = {1.,1.,1.,1.};
    GLfloat col[] = {0.,0.,0.,1.f};
    GLfloat blk[] = {0.,0.,0.,1.f};

    // TOP LIGHT
    glEnable(GL_LIGHT0);
    GLfloat qaLightPosition0[] = {.0,.0,5.,1.};
    glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition0);

    // TOP LIGHT
    glEnable(GL_LIGHT1);
    GLfloat qaLightPosition1[] = {.0,.0,-5.,1.};
    glLightfv(GL_LIGHT1, GL_AMBIENT, qaAmbientLight);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, qaDiffuseLight);
    glLightfv(GL_LIGHT1, GL_SPECULAR, qaSpecularLight);
    glLightfv(GL_LIGHT1, GL_POSITION, qaLightPosition1);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blk);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.);
}

void
morph::Gdisplay::redrawDisplay()
{
    glXMakeCurrent (this->disp, this->win, this->glc);
    XGetWindowAttributes (this->disp, this->win, &this->gwa);
    glViewport (0, 0, this->gwa.width, this->gwa.height);
    glXSwapBuffers (this->disp, this->win);
}

void
morph::Gdisplay::drawHex (double x, double y, double z, double r,
                          double red, double green, double blue)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = {(GLfloat)red, (GLfloat)green, (GLfloat)blue, 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    double ry = r * 1.154700538379252; // r * 1.0/sin(pi/3.0)
    double hry = ry * 0.5;
    glBegin(GL_POLYGON);
    glVertex3f(x,y+ry,z);
    glVertex3f(x+r,y+hry,z);
    glVertex3f(x+r,y-hry,z);
    glVertex3f(x,y-ry,z);
    glVertex3f(x-r,y-hry,z);
    glVertex3f(x-r,y+hry,z);
    glVertex3f(x,y+ry,z);
    glEnd();
}

void
morph::Gdisplay::drawHex (array<float,3> pos, float r, array<float,3> c)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = {(GLfloat)c[0], (GLfloat)c[1], (GLfloat)c[2], 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    float ry = r * 1.15470053838f; // r * 1.0/sin(pi/3.0)
    float hry = ry * 0.5f;
    glBegin(GL_POLYGON);
    // x is pos[0], y is pos[1], z is pos[2]
    glVertex3f(pos[0],pos[1]+ry,pos[2]);
    glVertex3f(pos[0]+r,pos[1]+hry,pos[2]);
    glVertex3f(pos[0]+r,pos[1]-hry,pos[2]);
    glVertex3f(pos[0],pos[1]-ry,pos[2]);
    glVertex3f(pos[0]-r,pos[1]-hry,pos[2]);
    glVertex3f(pos[0]-r,pos[1]+hry,pos[2]);
    glVertex3f(pos[0],pos[1]+ry,pos[2]);
    glEnd();
}

void
morph::Gdisplay::drawHex (array<float,3> pos, array<float,3> offset, float r, array<float,3> c)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = {(GLfloat)c[0], (GLfloat)c[1], (GLfloat)c[2], 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    float ry = r * 1.15470053838f; // r * 1.0/sin(pi/3.0)
    float hry = ry * 0.5f;
    glBegin(GL_POLYGON);
    // x is pos[0], y is pos[1], z is pos[2]
    glVertex3f(pos[0]+offset[0],pos[1]+offset[1]+ry,pos[2]+offset[2]);
    glVertex3f(pos[0]+offset[0]+r,pos[1]+offset[1]+hry,pos[2]+offset[2]);
    glVertex3f(pos[0]+offset[0]+r,pos[1]+offset[1]-hry,pos[2]+offset[2]);
    glVertex3f(pos[0]+offset[0],pos[1]+offset[1]-ry,pos[2]+offset[2]);
    glVertex3f(pos[0]+offset[0]-r,pos[1]+offset[1]-hry,pos[2]+offset[2]);
    glVertex3f(pos[0]+offset[0]-r,pos[1]+offset[1]+hry,pos[2]+offset[2]);
    glVertex3f(pos[0]+offset[0],pos[1]+offset[1]+ry,pos[2]+offset[2]);
    glEnd();
}

/// @param gray gray value from 0.0 to 1.0
/// @returns RGB value in jet colormap
array<float,3>
morph::Gdisplay::getJetColorF (double gray)
{
    float color_table[][3] = {
        {0.0, 0.0, 0.5}, // #00007F
        {0.0, 0.0, 1.0}, // blue
        {0.0, 0.5, 1.0}, // #007FFF
        {0.0, 1.0, 1.0}, // cyan
        {0.5, 1.0, 0.5}, // #7FFF7F
        {1.0, 1.0, 0.0}, // yellow
        {1.0, 0.5, 0.0}, // #FF7F00
        {1.0, 0.0, 0.0}, // red
        {0.5, 0.0, 0.0}, // #7F0000
    };

    array<float,3> col;
    float ivl = 1.0/8.0;
    for (int i=0; i<8; i++) {
        double llim = (i==0)?0.:(double)i/8.;
        double ulim = (i==7)?1.:((double)i+1.)/8.;
        if (gray >= llim && gray <= ulim) {
            for (int j=0; j<3; j++) {
                float c = static_cast<float>(gray - llim);
                col[j] = (color_table[i][j]*(ivl-c)/ivl + color_table[i+1][j]*c/ivl);
            }
            break;
        }
    }
    return col;
}

void
morph::Gdisplay::drawHex (array<float,3> pos, array<float,3> offset, float r, float val)
{
    array<float,3> c = morph::Gdisplay::getJetColorF (val);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = {(GLfloat)c[0], (GLfloat)c[1], (GLfloat)c[2], 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    float ry = r * 1.15470053838f; // r * 1.0/sin(pi/3.0)
    float hry = ry * 0.5f;
    glBegin(GL_POLYGON);
    // x is pos[0], y is pos[1], z is val
    glVertex3f(pos[0]+offset[0],pos[1]+offset[1]+ry,val+offset[2]);
    glVertex3f(pos[0]+offset[0]+r,pos[1]+offset[1]+hry,val+offset[2]);
    glVertex3f(pos[0]+offset[0]+r,pos[1]+offset[1]-hry,val+offset[2]);
    glVertex3f(pos[0]+offset[0],pos[1]+offset[1]-ry,val+offset[2]);
    glVertex3f(pos[0]+offset[0]-r,pos[1]+offset[1]-hry,val+offset[2]);
    glVertex3f(pos[0]+offset[0]-r,pos[1]+offset[1]+hry,val+offset[2]);
    glVertex3f(pos[0]+offset[0],pos[1]+offset[1]+ry,val+offset[2]);
    glEnd();
}

void
morph::Gdisplay::drawHexSeg (array<float,3> pos, array<float,3> offset, double r,
                             array<float,3> rgb, int edgeIndex)
{
    double ry = r * 1.154700538379252; // r * 1.0/sin(pi/3.0)
    double hry = ry * 0.5;

    double ax=0.0, ay=0.0, bx=0.0, by=0.0;
    switch (edgeIndex){
    case (0):{
        ax=pos[0]+r,    ay=pos[1]-hry;
        bx=pos[0]+r,    by=pos[1]+hry;
    }break;
    case (1):{
        ax=pos[0]+r,    ay=pos[1]+hry;
        bx=pos[0],      by=pos[1]+ry;
    }break;
    case (2):{
        ax=pos[0],      ay=pos[1]+ry;
        bx=pos[0]-r,    by=pos[1]+hry;
    }break;
    case (3):{
        ax=pos[0]-r,    ay=pos[1]+hry;
        bx=pos[0]-r,    by=pos[1]-hry;
    }break;
    case (4):{
        ax=pos[0]-r,    ay=pos[1]-hry;
        bx=pos[0],      by=pos[1]-ry;
    }break;
    case (5):{
        ax=pos[0],      ay=pos[1]-ry;
        bx=pos[0]+r,    by=pos[1]-hry;
    }break;
    }

    GLfloat col[] = {(GLfloat)rgb[0], (GLfloat)rgb[1], (GLfloat)rgb[2], 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glPointSize(5.);
    glBegin(GL_LINES);
    glVertex3d(ax+offset[0],ay+offset[1],pos[2]+offset[2]);
    glVertex3d(bx+offset[0],by+offset[1],pos[2]+offset[2]);
    glEnd();
}

 void
 morph::Gdisplay::drawRect(double x, double y, double z, double width, double height, vector<double> color){
    double halfWidth = width*0.5;
    double halfHeight = height*0.5;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = {(GLfloat)color[0], (GLfloat)color[1], (GLfloat)color[2], 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glBegin(GL_POLYGON);
    glVertex3d(x-halfWidth,y-halfHeight,z);
    glVertex3d(x-halfWidth,y+halfHeight,z);
    glVertex3d(x+halfWidth,y+halfHeight,z);
    glVertex3d(x+halfWidth,y-halfHeight,z);
    glEnd();
}

void
morph::Gdisplay::drawTri (std::vector<double> p1,
                          std::vector<double> p2,
                          std::vector<double> p3,
                          std::vector<double> C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    GLfloat col[] = { (GLfloat)C[0], (GLfloat)C[1], (GLfloat)C[2], 1.0f };
    glMaterialfv (GL_FRONT, GL_DIFFUSE, col);
    glBegin (GL_TRIANGLES);
    glVertex3f (p1[0], p1[1], p1[2]);
    glVertex3f (p2[0], p2[1], p2[2]);
    glVertex3f (p3[0], p3[1], p3[2]);
    glEnd();
}

void
morph::Gdisplay::drawTriFill (std::vector<double> p1,
                              std::vector<double> p2,
                              std::vector<double> p3,
                              std::vector<double> C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = { (GLfloat)C[0], (GLfloat)C[1], (GLfloat)C[2], 1.0f };
    glMaterialfv (GL_FRONT, GL_DIFFUSE, col);
    glBegin (GL_TRIANGLES);
    glVertex3f (p1[0], p1[1], p1[2]);
    glVertex3f (p2[0], p2[1], p2[2]);
    glVertex3f (p3[0], p3[1], p3[2]);
    glEnd();
}

void
morph::Gdisplay::drawTriFill (std::array<float,3> p1,
                              std::array<float,3> p2,
                              std::array<float,3> p3,
                              std::array<float, 3> C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = { (GLfloat)C[0], (GLfloat)C[1], (GLfloat)C[2], 1.0f };
    glMaterialfv (GL_FRONT, GL_DIFFUSE, col);
    glBegin (GL_TRIANGLES);
    glVertex3f (p1[0], p1[1], p1[2]);
    glVertex3f (p2[0], p2[1], p2[2]);
    glVertex3f (p3[0], p3[1], p3[2]);
    glEnd();
}

void
morph::Gdisplay::drawSphere (double x,double y,double z,double r,std::vector<double> C,int res)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat col[] = {(GLfloat)C[0], (GLfloat)C[1], (GLfloat)C[2], 1.f};
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);
    static GLUquadricObj* Sphere = gluNewQuadric();
    glPushMatrix();
    glTranslatef(x,y,z);
    gluSphere(Sphere, r, res, res);
    glPopMatrix();
};

void
morph::Gdisplay::drawLine (double ax, double ay, double az,
                           double bx, double by, double bz,
                           double red, double green, double blue, double width)
{
    glColor3f(red,green,blue);
    glPointSize(width);
    glBegin(GL_LINES);
    glVertex3d(ax,ay,az);
    glVertex3d(bx,by,bz);
    glEnd();
};

void
morph::Gdisplay::addFloor (double x, double y)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLfloat col[] = {0.92,0.94,0.96, 1.f};
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);
    glBegin(GL_QUADS);
    glVertex3d(-x,-y,0.0);
    glVertex3d(-x,+y,0.0);
    glVertex3d(+x,+y,0.0);
    glVertex3d(+x,-y,0.0);
    glNormal3d(0.,0.,1.);
    glEnd();
}

void
morph::Gdisplay::drawCylinder (float x1, float y1, float z1, float x2, float y2, float z2, float radA, float radB, int subdivisions, std::vector<double> col)
{
    GLUquadricObj *quadric=gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat colr[] = {(GLfloat)col[0], (GLfloat)col[1], (GLfloat)col[2], 1.f};
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);

    double px = x1-x2;
    double py = y1-y2;
    double pz = z1-z2;
    double len = sqrt(px*px+py*py+pz*pz);

    glPushMatrix();

    if (len>0.&&radB>0.&&radA>0.){

        glTranslatef(x2,y2,z2);
        glRotatef((180./M_PI)*acos(pz/len),-py+1e-6,+px,0.);

        gluQuadricOrientation(quadric,GLU_OUTSIDE);
        gluCylinder(quadric, radB, radA, len, subdivisions, 1);

        gluQuadricOrientation(quadric,GLU_INSIDE);
        gluDisk( quadric, 0.0, radB, subdivisions, 1);
        glTranslatef( 0,0,len );

        gluQuadricOrientation(quadric,GLU_OUTSIDE);
        gluDisk( quadric, 0.0, radA, subdivisions, 1);

    }
    glPopMatrix();
}

void
morph::Gdisplay::drawMesh (std::vector<std::vector<std::vector<double> > > X,
                           std::vector<std::vector<std::vector<double> > > C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);

    // obviously this will come out!
    int iN = X.size();
    int jN = X[0].size();

    for (int i=1;i<iN;i++){
        for (int j=1;j<jN;j++){
            GLfloat colr[]={(GLfloat)C[i][j][0],(GLfloat)C[i][j][1],(GLfloat)C[i][j][2],1.f};
            glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
            glBegin(GL_QUADS);
            double ax = X[i][j-1][0]-X[i-1][j-1][0];
            double ay = X[i][j-1][1]-X[i-1][j-1][1];
            double az = X[i][j-1][2]-X[i-1][j-1][2];
            double bx = X[i-1][j][0]-X[i-1][j-1][0];
            double by = X[i-1][j][1]-X[i-1][j-1][1];
            double bz = X[i-1][j][2]-X[i-1][j-1][2];
            double nx = ay*bz-az*by;
            double ny = az*bx-ax*bz;
            double nz = ax*by-ay*bx;
            double nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
            glNormal3d(nx*nl,ny*nl,nz*nl);
            glVertex3d(X[i-1][j-1][0],X[i-1][j-1][1],X[i-1][j-1][2]);
            glVertex3d(X[i-1][j][0],X[i-1][j][1],X[i-1][j][2]);
            glVertex3d(X[i][j][0],X[i][j][1],X[i][j][2]);
            glVertex3d(X[i][j-1][0],X[i][j-1][1],X[i][j-1][2]);
            glEnd();

        }

        GLfloat colr[]={(GLfloat)C[i][0][0],(GLfloat)C[i][0][1],(GLfloat)C[i][0][2],1.f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
        glBegin(GL_QUADS);
        double ax = X[i][jN-1][0]-X[i-1][jN-1][0];
        double ay = X[i][jN-1][1]-X[i-1][jN-1][1];
        double az = X[i][jN-1][2]-X[i-1][jN-1][2];
        double bx = X[i-1][0][0]-X[i-1][jN-1][0];
        double by = X[i-1][0][1]-X[i-1][jN-1][1];
        double bz = X[i-1][0][2]-X[i-1][jN-1][2];
        double nx = ay*bz-az*by;
        double ny = az*bx-ax*bz;
        double nz = ax*by-ay*bx;
        double nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[i-1][jN-1][0],X[i-1][jN-1][1],X[i-1][jN-1][2]);
        glVertex3d(X[i-1][0][0],X[i-1][0][1],X[i-1][0][2]);
        glVertex3d(X[i][0][0],X[i][0][1],X[i][0][2]);
        glVertex3d(X[i][jN-1][0],X[i][jN-1][1],X[i][jN-1][2]);
        glEnd();

    }
}

void
morph::Gdisplay::drawMesh2 (std::vector<std::vector<std::vector<double> > > X, std::vector<double> col)
{
    //GLUquadricObj *quadric=gluNewQuadric();
    //gluQuadricNormals(quadric, GLU_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat colr[] = {(GLfloat)col[0], (GLfloat)col[1], (GLfloat)col[2], 1.f};
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);

    // obviously this will come out!
    int iN = X.size();
    int jN = X[0].size();

    for (int i=1;i<iN;i++){
        for (int j=1;j<jN;j++){
            glBegin(GL_QUADS);
            double ax = X[i][j-1][0]-X[i-1][j-1][0];
            double ay = X[i][j-1][1]-X[i-1][j-1][1];
            double az = X[i][j-1][2]-X[i-1][j-1][2];
            double bx = X[i-1][j][0]-X[i-1][j-1][0];
            double by = X[i-1][j][1]-X[i-1][j-1][1];
            double bz = X[i-1][j][2]-X[i-1][j-1][2];
            double nx = ay*bz-az*by;
            double ny = az*bx-ax*bz;
            double nz = ax*by-ay*bx;
            double nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
            glNormal3d(nx*nl,ny*nl,nz*nl);
            glVertex3d(X[i-1][j-1][0],X[i-1][j-1][1],X[i-1][j-1][2]);
            glVertex3d(X[i-1][j][0],X[i-1][j][1],X[i-1][j][2]);
            glVertex3d(X[i][j][0],X[i][j][1],X[i][j][2]);
            glVertex3d(X[i][j-1][0],X[i][j-1][1],X[i][j-1][2]);
            glEnd();

        }

        glBegin(GL_QUADS);
        double ax = X[i][jN-1][0]-X[i-1][jN-1][0];
        double ay = X[i][jN-1][1]-X[i-1][jN-1][1];
        double az = X[i][jN-1][2]-X[i-1][jN-1][2];
        double bx = X[i-1][0][0]-X[i-1][jN-1][0];
        double by = X[i-1][0][1]-X[i-1][jN-1][1];
        double bz = X[i-1][0][2]-X[i-1][jN-1][2];
        double nx = ay*bz-az*by;
        double ny = az*bx-ax*bz;
        double nz = ax*by-ay*bx;
        double nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[i-1][jN-1][0],X[i-1][jN-1][1],X[i-1][jN-1][2]);
        glVertex3d(X[i-1][0][0],X[i-1][0][1],X[i-1][0][2]);
        glVertex3d(X[i][0][0],X[i][0][1],X[i][0][2]);
        glVertex3d(X[i][jN-1][0],X[i][jN-1][1],X[i][jN-1][2]);
        glEnd();
    }
}

void
morph::Gdisplay::drawTorus (std::vector<std::vector<std::vector<double> > > X,
                            std::vector<std::vector<std::vector<double> > > C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);

    // obviously this will come out!
    int iN = X.size();
    int jN = X[0].size();

    for (int i=1;i<=iN;i++){
        int I0 = i%iN;
        int I1 = (i-1)%iN;
        for (int j=1;j<=jN;j++){
            int J0 = j%jN;
            int J1 = (j-1)%jN;
            //GLfloat colr[]={C[I0][J0][0],C[I0][J0][1],C[I0][J0][2],1.f};
            GLfloat colr[]={0.,1.,0.5,1.f};
            glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
            glBegin(GL_QUADS);

            double ax = X[I0][J1][0]-X[I1][J1][0];
            double ay = X[I0][J1][1]-X[I1][J1][1];
            double az = X[I0][J1][2]-X[I1][J1][2];
            double bx = X[I1][J0][0]-X[I1][J1][0];
            double by = X[I1][J0][1]-X[I1][J1][1];
            double bz = X[I1][J0][2]-X[I1][J1][2];
            double nx = ay*bz-az*by;
            double ny = az*bx-ax*bz;
            double nz = ax*by-ay*bx;
            double nl = 1./sqrt(nx*nx+ny*ny+nz*nz);
            nx*=nl;
            ny*=nl;
            nz*=nl;

            if((C[I0][J0][0]*nx+C[I0][J0][1]*ny+C[I0][J0][2]*nz)>0.){
                glNormal3d(nx,ny,nz);
            } else {
                glNormal3d(-nx,-ny,-nz);
            }

            //glNormal3d(C[I0][J0][0],C[I0][J0][1],C[I0][J0][2]);//nx*nl,ny*nl,nz*nl);

            glVertex3d(X[I1][J1][0],X[I1][J1][1],X[I1][J1][2]);
            glVertex3d(X[I1][J0][0],X[I1][J0][1],X[I1][J0][2]);
            glVertex3d(X[I0][J0][0],X[I0][J0][1],X[I0][J0][2]);
            glVertex3d(X[I0][J1][0],X[I0][J1][1],X[I0][J1][2]);

            glEnd();
        }
    }
}

void
morph::Gdisplay::drawCubeSphere (std::vector<std::vector<double> > X)
{
    GLfloat colrZip[]={1.,1.,1.,1.f};
    GLfloat colrTri[]={1.,1.,1.,1.f};

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);

    int N = X.size();
    int n = sqrt(N/6);
    int k=(n-1);
    int nk=n*k;
    int n2 = n*n;
    int a, b, c, d;
    double ax, ay, az, bx, by, bz, nx, ny, nz, nl;
    for(int i=0;i<6;i++){
        for(int s=1;s<n;s++){
            for(int t=1;t<n;t++){
                GLfloat colr[]={0.,1.,0.5,1.f};
                glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
                glBegin(GL_QUADS);

                a = i*n2+(s-1)*n+(t-1);
                b = i*n2+(s-1)*n+(t);
                c = i*n2+(s)*n+(t);
                d = i*n2+(s)*n+(t-1);

                ax = X[b][0]-X[a][0];
                ay = X[b][1]-X[a][1];
                az = X[b][2]-X[a][2];
                bx = X[d][0]-X[a][0];
                by = X[d][1]-X[a][1];
                bz = X[d][2]-X[a][2];
                nx = ay*bz-az*by;
                ny = az*bx-ax*bz;
                nz = ax*by-ay*bx;
                nl = -1./sqrt(nx*nx+ny*ny+nz*nz);

                glNormal3d(nx*nl,ny*nl,nz*nl);

                glVertex3d(X[a][0],X[a][1],X[a][2]);
                glVertex3d(X[b][0],X[b][1],X[b][2]);
                glVertex3d(X[c][0],X[c][1],X[c][2]);
                glVertex3d(X[d][0],X[d][1],X[d][2]);

                glEnd();
            }
        }
    }

    //ZIPPING
    for(int i=1;i<n;i++){

        glMaterialfv(GL_FRONT, GL_DIFFUSE, colrZip);
        glBegin(GL_QUADS);

        //0A->2C
        a=0*n2+(i-1)*n;
        b=0*n2+i*n;
        c=2*n2+i;
        d=2*n2+(i-1);

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //0B->3A
        a=0*n2+k+n*(i-1);
        b=3*n2+(i-1)*n;
        c=3*n2+i*n;
        d=0*n2+k+n*i;

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz); //CHANGED TO -
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //0C->4A
        a=0*n2+(i-1);
        b=4*n2+(i-1)*n;
        c=4*n2+i*n;
        d=0*n2+i;

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz); // CHANGED TO -
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //0D->5C
        a=0*n2+nk+(i-1);
        b=0*n2+nk+i;
        c=5*n2+i;
        d=5*n2+(i-1);

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //1A->4B
        a=1*n2+(i-1)*n;
        b=1*n2+i*n;
        c=4*n2+k+n*i;
        d=4*n2+k+n*(i-1);

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //1B->5D
        a=1*n2+k+n*(i-1);
        b=5*n2+nk+(i-1);
        c=5*n2+nk+i;
        d=1*n2+k+n*i;


        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz); // CHANGED TO -
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //1C->2D
        a=1*n2+(i-1);
        b=2*n2+nk+(i-1);
        c=2*n2+nk+i;
        d=1*n2+i;

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz); // CHANGED TO -
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //1D->3B
        a=1*n2+nk+(i-1);
        b=1*n2+nk+i;
        c=3*n2+k+n*i;
        d=3*n2+k+n*(i-1);
        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //2A->4C
        a=2*n2+(i-1)*n;
        b=2*n2+i*n;
        c=4*n2+i;
        d=4*n2+(i-1);
        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //2B->5A
        a=2*n2+k+n*(i-1);
        b=5*n2+(i-1)*n;
        c=5*n2+i*n;
        d=2*n2+k+n*i;

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);    // CHANGED TO -
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //3C->4D
        a=3*n2+(i-1);
        b=4*n2+nk+(i-1);
        c=4*n2+nk+i;
        d=3*n2+i;

        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);   // CHANGED TO -
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);

        //3D->5B
        a=3*n2+nk+(i-1);
        b=3*n2+nk+i;
        c=5*n2+n*i+k;
        d=5*n2+n*(i-1)+k;
        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);
        glEnd();
    }

    // CORNER TRIANGLES

    glMaterialfv(GL_FRONT, GL_DIFFUSE, colrTri);
    glBegin(GL_QUADS);

    a=0*n2+nk;
    b=5*n2;
    d=2*n2+k;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz); // CHANGED TO -
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);

    a=0*n2+nk+k;
    b=3*n2+nk;
    d=5*n2+k;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);


    a=0*n2;
    b=2*n2;
    d=4*n2;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);

    a=0*n2+k;
    b=4*n2+nk;
    d=3*n2;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);   // CHANGED TO -
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);


    // BOTTON TRIANGLES

    a=1*n2+k;
    b=5*n2+nk;
    d=2*n2+nk+k;
    c=b;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);  // CHANGED TO -
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[c][0],X[c][1],X[c][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);


    a=1*n2+nk+k;
    b=3*n2+nk+k;
    d=5*n2+nk+k;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);


    a=1*n2;
    b=2*n2+nk;
    d=4*n2+k;
    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);

    a=1*n2+nk;
    b=4*n2+nk+k;
    d=3*n2+k;

    ax = X[b][0]-X[a][0];
    ay = X[b][1]-X[a][1];
    az = X[b][2]-X[a][2];
    bx = X[d][0]-X[a][0];
    by = X[d][1]-X[a][1];
    bz = X[d][2]-X[a][2];
    nx = ay*bz-az*by;
    ny = az*bx-ax*bz;
    nz = ax*by-ay*bx;
    nl = -1./sqrt(nx*nx+ny*ny+nz*nz);   // CHANGED TO -
    glNormal3d(nx*nl,ny*nl,nz*nl);
    glVertex3d(X[a][0],X[a][1],X[a][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[b][0],X[b][1],X[b][2]);
    glVertex3d(X[d][0],X[d][1],X[d][2]);
    glEnd();
}

void
morph::Gdisplay::drawSphereFromMesh (std::vector<std::vector<double> > X,
                                     std::vector<std::vector<int> > M,
                                     std::vector<std::vector<double> > C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    GLfloat wht[] = {1.f, 1.f, 1.f, 1.f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, wht);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);

    int a, b, c, d;
    double ax, ay, az, bx, by, bz, nx, ny, nz, nl;

    for(unsigned int i=0;i<X.size();i++){
        GLfloat col[]={(GLfloat)C[i][0],(GLfloat)C[i][1],(GLfloat)C[i][2],1.f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
        glBegin(GL_QUADS);
        a = i;
        b = M[i][0];        //+x
        c = M[M[i][0]][1];  //+xy
        d = M[i][1];        //+y
        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = 1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);
        glEnd();
    }
    for(unsigned int i=0;i<X.size();i++){
        GLfloat col[]={(GLfloat)C[i][0],(GLfloat)C[i][1],(GLfloat)C[i][2],1.f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
        glBegin(GL_QUADS);
        a = i;
        b = M[i][2];        //+x
        c = M[M[i][2]][3];  //+xy
        d = M[i][3];        //+y
        ax = X[b][0]-X[a][0];
        ay = X[b][1]-X[a][1];
        az = X[b][2]-X[a][2];
        bx = X[d][0]-X[a][0];
        by = X[d][1]-X[a][1];
        bz = X[d][2]-X[a][2];
        nx = ay*bz-az*by;
        ny = az*bx-ax*bz;
        nz = ax*by-ay*bx;
        nl = 1./sqrt(nx*nx+ny*ny+nz*nz);
        glNormal3d(nx*nl,ny*nl,nz*nl);
        glVertex3d(X[a][0],X[a][1],X[a][2]);
        glVertex3d(X[b][0],X[b][1],X[b][2]);
        glVertex3d(X[c][0],X[c][1],X[c][2]);
        glVertex3d(X[d][0],X[d][1],X[d][2]);
        glEnd();
    }
}

void
morph::Gdisplay::drawFlatCube (std::vector<std::vector<int> > C,
                               std::vector<std::vector<double> > Col,
                               double X, double Y, double Z)
{
    int n = sqrt(C.size()/6);
    double dn1 = 1./(double)n;
    double dn2 = 0.5*dn1;
    double scale = 0.5;
    double x, y, xp, yp;
    std::vector<double> xoff(6);
    std::vector<double> yoff(6);
    xoff[0]=  0; yoff[0]= 1.5;
    xoff[1]=  0; yoff[1]= 0.5;
    xoff[2]=  0; yoff[2]=-0.5;
    xoff[3]=  0; yoff[3]= -1.5;
    xoff[4]= -1; yoff[4]= 0.5;
    xoff[5]= +1; yoff[5]= 0.5;

    for(unsigned int i=0;i<C.size();i++){
        int f = C[i][0];
        x = xoff[f]+((((double)C[i][1]+0.5)*dn1)-0.5)-dn2;
        xp = x+dn1;
        x*=scale;
        xp*=scale;

        y = yoff[f]+((((double)C[i][2]+0.5)*dn1)-0.5)-dn2;
        yp = y+dn1;
        y*=scale;
        yp*=scale;

        GLfloat colr[]={(GLfloat)Col[i][0],(GLfloat)Col[i][1],(GLfloat)Col[i][2],1.f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, colr);
        glBegin(GL_QUADS);
        glNormal3d(0,0,1);
        glVertex3d(X+x,Y+y,Z+0);
        glVertex3d(X+x,Y+yp,Z+0);
        glVertex3d(X+xp,Y+yp,Z+0);
        glVertex3d(X+xp,Y+y,Z+0);
        glEnd();
    }
}

void
morph::Gdisplay::addQuad (std::vector<double> p1,
                          std::vector<double> p2,
                          std::vector<double> p3,
                          std::vector<double> p4,
                          std::vector<double> N,
                          std::vector<double> C)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill
    glBegin(GL_POLYGON);
    GLfloat col[] = {(GLfloat)C[0], (GLfloat)C[1], (GLfloat)C[2], 1.f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.);
    glNormal3d(N[0],N[1],N[2]);
    glVertex3d(p1[0],p1[1],p1[2]);
    glVertex3d(p2[0],p2[1],p2[2]);
    glVertex3d(p3[0],p3[1],p3[2]);
    glVertex3d(p4[0],p4[1],p4[2]);
    glEnd();
}

void
morph::Gdisplay::addCrossHairs (double d, double l, int w)
{
    //x col
    drawLine(-d,-d,-d,        -d+l,-d,-d,                1.,0.,0.,w); //-z
    drawLine(-d,+d,-d,        -d+l,+d,-d,                1.,0.,0.,w);
    drawLine(+d,+d,-d,        +d-l,+d,-d,                1.,0.,0.,w);
    drawLine(+d,-d,-d,        +d-l,-d,-d,                1.,0.,0.,w);
    drawLine(-d,-d,0.,        -d+l,-d,0.,                1.,0.,0.,w); //0z
    drawLine(-d,+d,0.,        -d+l,+d,0.,                1.,0.,0.,w);
    drawLine(+d,+d,0.,        +d-l,+d,0.,                1.,0.,0.,w);
    drawLine(+d,-d,0.,        +d-l,-d,0.,                1.,0.,0.,w);
    drawLine(-d,-d,+d,        -d+l,-d,+d,                1.,0.,0.,w); //+z
    drawLine(-d,+d,+d,        -d+l,+d,+d,                1.,0.,0.,w);
    drawLine(+d,+d,+d,        +d-l,+d,+d,                1.,0.,0.,w);
    drawLine(+d,-d,+d,        +d-l,-d,+d,                1.,0.,0.,w);

    //y col
    drawLine(-d,-d,-d,        -d,-d+l,-d,                0.,1.,0.,w); //-z
    drawLine(-d,+d,-d,        -d,+d-l,-d,                0.,1.,0.,w);
    drawLine(+d,+d,-d,        +d,+d-l,-d,                0.,1.,0.,w);
    drawLine(+d,-d,-d,        +d,-d+l,-d,                0.,1.,0.,w);
    drawLine(-d,-d,0.,        -d,-d+l,0.,                0.,1.,0.,w); //0z
    drawLine(-d,+d,0.,        -d,+d-l,0.,                0.,1.,0.,w);
    drawLine(+d,+d,0.,        +d,+d-l,0.,                0.,1.,0.,w);
    drawLine(+d,-d,0.,        +d,-d+l,0.,                0.,1.,0.,w);
    drawLine(-d,-d,+d,        -d,-d+l,+d,                0.,1.,0.,w); //+z
    drawLine(-d,+d,+d,        -d,+d-l,+d,                0.,1.,0.,w);
    drawLine(+d,+d,+d,        +d,+d-l,+d,                0.,1.,0.,w);
    drawLine(+d,-d,+d,        +d,-d+l,+d,                0.,1.,0.,w);

    //z col
    drawLine(-d,-d,-d,        -d,-d,-d+l,                0.,0.,1.,w); //-z
    drawLine(-d,+d,-d,        -d,+d,-d+l,                0.,0.,1.,w);
    drawLine(+d,+d,-d,        +d,+d,-d+l,                0.,0.,1.,w);
    drawLine(+d,-d,-d,        +d,-d,-d+l,                0.,0.,1.,w);
    drawLine(-d,-d,0.,        -d,-d,0.+l,                0.,0.,1.,w); //-z
    drawLine(-d,+d,0.,        -d,+d,0.+l,                0.,0.,1.,w);
    drawLine(+d,+d,0.,        +d,+d,0.+l,                0.,0.,1.,w);
    drawLine(+d,-d,0.,        +d,-d,0.+l,                0.,0.,1.,w);
    drawLine(-d,-d,+d,        -d,-d,+d-l,                0.,0.,1.,w); //+z
    drawLine(-d,+d,+d,        -d,+d,+d-l,                0.,0.,1.,w);
    drawLine(+d,+d,+d,        +d,+d,+d-l,                0.,0.,1.,w);
    drawLine(+d,-d,+d,        +d,-d,+d-l,                0.,0.,1.,w);
}

void
morph::Gdisplay::saveImage (std::string filename)
{
    glXMakeCurrent(disp, win, glc);
    GLubyte * bits; //RGB bits
    GLint viewport[4]; //current viewport
    glGetIntegerv(GL_VIEWPORT, viewport);
    int w = viewport[2];
    int h = viewport[3];
    bits = new GLubyte[w*3*h];
    glFinish(); //finish all commands of OpenGL
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glReadPixels(0, 0, w, h, GL_BGR_EXT, GL_UNSIGNED_BYTE, bits);
    cv::Mat capImg (h, w, CV_8UC3); // 3 channels, 8 bits
    cv::Vec3b triplet;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            triplet[0] = (unsigned char)(bits[(h-i-1)*3*w + j*3+0]);
            triplet[1] = (unsigned char)(bits[(h-i-1)*3*w + j*3+1]);
            triplet[2] = (unsigned char)(bits[(h-i-1)*3*w + j*3+2]);
            capImg.at<cv::Vec3b>(i,j) = triplet;
        }
    }
    imwrite (filename.c_str(), capImg);
    delete[] bits;
}
