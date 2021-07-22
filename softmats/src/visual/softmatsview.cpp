#ifdef USE_GLEW
#include <GL/glew.h>
#endif
#include "softmatsview.h"
#include "../util/openglutils.h"
#include "../util/config.h"

#ifdef __OSX__
# include <OpenGL/gl3.h>
//#else
//# include <GL3/gl3.h>
#endif

using namespace morph::softmats;

void SoftmatsView::setupGround( Body *ground){
    std::vector<Face *>& faces = ground->getMesh()->getFaces();
    //std::vector<Point *>& vert = ground->getMesh()->getVertices();

    std::vector<float> pvalues;
    std::vector<float> tvalues;
    std::vector<float> nvalues;
    int i;

    for( Face* f : faces ){
        for(i=0; i<3; ++i)pvalues.push_back(f->points[0]->x(i));
        for(i=0; i<3; ++i)pvalues.push_back(f->points[1]->x(i));
        for(i=0; i<3; ++i)pvalues.push_back(f->points[2]->x(i));

        tvalues.push_back(f->points[0]->uv[0]);
        tvalues.push_back(f->points[0]->uv[1]);
        tvalues.push_back(f->points[1]->uv[0]);
        tvalues.push_back(f->points[1]->uv[1]);
        tvalues.push_back(f->points[2]->uv[0]);
        tvalues.push_back(f->points[2]->uv[1]);

        for(i=0; i<3; ++i)nvalues.push_back(f->points[0]->normal(i));
        for(i=0; i<3; ++i)nvalues.push_back(f->points[1]->normal(i));
        for(i=0; i<3; ++i)nvalues.push_back(f->points[2]->normal(i));
    }

    // vertices
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glBufferData(GL_ARRAY_BUFFER, pvalues.size()*4, &pvalues[0], GL_STATIC_DRAW );
    // texture
    glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
    glBufferData(GL_ARRAY_BUFFER, tvalues.size()*4, &tvalues[0], GL_STATIC_DRAW );
    // normals
    glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );
    glBufferData(GL_ARRAY_BUFFER, nvalues.size()*4, &nvalues[0], GL_STATIC_DRAW );
}

void SoftmatsView::setup(){
    glGenVertexArrays( numVAOs, vao );
    glBindVertexArray( vao[0] );
    glGenBuffers( numVBOs, vbo );
}

void SoftmatsView::init( ){
    std::cout << "Initializing window\n";
#ifdef USE_GLEW
    glewExperimental = true;
#endif
    if( !glfwInit() ){ exit(EXIT_FAILURE); }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    window = glfwCreateWindow( 600, 600, "Soft body simulator", NULL, NULL );
    glfwMakeContextCurrent( window );
    std::cout << "Window created\n";
#ifdef USE_GLEW
    if( glewInit() != GLEW_OK ){ exit(EXIT_FAILURE); }
    std::cout << "Glew initialized\n";
#endif
    glfwSwapInterval( 1 );
    std::string vshader = Config::getConfig()->getShaderLocation() + "softmats.vsh";
    std::string fshader = Config::getConfig()->getShaderLocation() + "softmats.fsh";
    renderingProgram = OpenglUtils::createShaderProgram(vshader.c_str(), fshader.c_str());
    OpenglUtils::checkOpenGLError();
    std::cout << "Shaders loaded\n";
    camera = { 0.0f, -0.5f, 10.5f };
    viewPort.x = 0.0f; viewPort.y = -2.0f; viewPort.z = 0.0f;
    light.initialLightLoc = { 5.0f, 2.0f, 2.0f };
    // White light
    light.globalAmbient[0] = 0.9f;
    light.globalAmbient[1] = 0.9f;
    light.globalAmbient[2] = 0.9f;
    light.globalAmbient[3] = 1.0f;
    light.lightAmbient[0] = 0.0f;
    light.lightAmbient[1] = 0.0f;
    light.lightAmbient[2] = 0.0f;
    light.lightAmbient[3] = 1.0f;
    light.ligthDiffuse[0] = 1.0f;
    light.ligthDiffuse[1] = 1.0f;
    light.ligthDiffuse[2] = 1.0f;
    light.ligthDiffuse[3] = 1.0f;
    light.lightSpecular[0] = 1.0f;
    light.lightSpecular[1] = 1.0f;
    light.lightSpecular[2] = 1.0f;
    light.lightSpecular[3] = 1.0f;
    // textureId = Utils::loadTextureImage("../res/fabric.jpg");
    textureId = OpenglUtils::loadTextureChecker( 1000, 1000 );
    setup();
}

void SoftmatsView::installLights( Body *b, morph::TransformMatrix<float>& vMatrix ){
    float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    light.posV = (vMatrix * light.currentPos).less_one_dim();

    // Get some locations
    light.globalAmbLoc = glGetUniformLocation( renderingProgram, "globalAmbient" );
    light.ambLoc = glGetUniformLocation( renderingProgram, "light.ambient" );
    light.diffLoc = glGetUniformLocation( renderingProgram, "light.diffuse" );
    light.specLoc = glGetUniformLocation( renderingProgram, "light.specular" );
    light.posLoc = glGetUniformLocation( renderingProgram, "light.position" );
    light.mAmbLoc = glGetUniformLocation( renderingProgram, "material.ambient" );
    light.mDiffLoc = glGetUniformLocation( renderingProgram, "material.diffuse" );
    light.mSpecLoc = glGetUniformLocation( renderingProgram, "material.specular" );
    light.mShiLoc = glGetUniformLocation( renderingProgram, "material.shininess" );

    // Set the uniforms in the shader
    glProgramUniform4fv( renderingProgram, light.globalAmbLoc, 1, light.globalAmbient );
    glProgramUniform4fv( renderingProgram, light.ambLoc, 1, light.lightAmbient );
    glProgramUniform4fv( renderingProgram, light.diffLoc, 1, light.ligthDiffuse );
    glProgramUniform4fv( renderingProgram, light.specLoc, 1, light.lightSpecular );
    glProgramUniform4fv( renderingProgram, light.posLoc, 1, light.posV.data() );

    if( b != NULL ){
        glProgramUniform4fv( renderingProgram, light.mAmbLoc, 1, b->material.matAmb );
        glProgramUniform4fv( renderingProgram, light.mDiffLoc, 1, b->material.matDif );
        glProgramUniform4fv( renderingProgram, light.mSpecLoc, 1, b->material.matSpe );
        glProgramUniform1f( renderingProgram, light.mShiLoc, b->material.matShi );
    }else{
        glProgramUniform4fv( renderingProgram, light.mAmbLoc, 1, zero );
        glProgramUniform4fv( renderingProgram, light.mDiffLoc, 1, zero );
        glProgramUniform4fv( renderingProgram, light.mSpecLoc, 1, zero );
        glProgramUniform1f( renderingProgram, light.mShiLoc, 0.0f );
    }
}


void SoftmatsView::preDisplay( ){

    glClear( GL_DEPTH_BUFFER_BIT );
    // glClearColor( 0.0, 0.18, 0.3, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT );
    glUseProgram( renderingProgram );

    viewPort.mvLoc = glGetUniformLocation( renderingProgram, "mv_matrix" );
    viewPort.prLoc = glGetUniformLocation( renderingProgram, "proj_matrix" );
    nLoc = glGetUniformLocation( renderingProgram, "norm_matrix" );
    typeLoc = glGetUniformLocation( renderingProgram, "type" );
    // Build perspective matrix
    glfwGetFramebufferSize( window, &viewPort.width, &viewPort.height );
    viewPort.aspect = (float)viewPort.width/(float)viewPort.height;
    viewPort.pMat.setToIdentity();
    viewPort.pMat.perspective( 60.0f, viewPort.aspect, 0.1f, 1000.0f ); // 60 degrees is 1.0472 rads
    viewPort.vMat.setToIdentity();
    viewPort.vMat.translate( -camera );
}

void SoftmatsView::displayGround(){
    mMat.setToIdentity();
    mvMat = viewPort.vMat*mMat;
    invTrMat = mvMat.invert();
    invTrMat.transpose();
    // Setup light
    light.currentPos = light.initialLightLoc;
    installLights(NULL, viewPort.vMat);

    // Copy matrices into uniform variables
    glUniformMatrix4fv( viewPort.mvLoc, 1, GL_FALSE, mvMat.mat.data() );
    glUniformMatrix4fv( viewPort.prLoc, 1, GL_FALSE, viewPort.pMat.mat.data() );
    glUniformMatrix4fv( nLoc, 1, GL_FALSE, invTrMat.mat.data());
    glUniform1i( typeLoc, 0 );
    //Associate VBO with the corresponding vertex attribute in the vertex shader
    glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );
    // Sending texture data
    glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 1 );
    // Send normals
    glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 2 );

    // Adjust settings
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureId );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
}

void SoftmatsView::displayBody( Body* b ){
    std::vector<Face *>& faces = b->getMesh()->getFaces();
    //std::vector<Point *>& vert = b->getMesh()->getVertices();

    std::vector<float> pvalues;
    std::vector<float> tvalues;
    std::vector<float> nvalues;
    int i;

    for( Face* f : faces ){
        // std::cout << "Face: " << f.p1 << ", " << f.p2 << ", " << f.p3 << "\n";
        //std::cout << "x: " << vert[f.p3].x(0) << ", y: " << vert[f.p3].x(1) <<", z: " << vert[f.p3].x(2) << std::endl;
        for(i=0; i<3; ++i)pvalues.push_back(f->points[0]->x(i));
        for(i=0; i<3; ++i)pvalues.push_back(f->points[1]->x(i));
        for(i=0; i<3; ++i)pvalues.push_back(f->points[2]->x(i));

        tvalues.push_back(f->points[0]->uv[0]); // was .s when uv was a ::vec2
        tvalues.push_back(f->points[0]->uv[1]);
        tvalues.push_back(f->points[1]->uv[0]);
        tvalues.push_back(f->points[1]->uv[1]);
        tvalues.push_back(f->points[2]->uv[0]);
        tvalues.push_back(f->points[2]->uv[1]);

        for(i=0; i<3; ++i)nvalues.push_back(f->points[0]->normal(i));
        for(i=0; i<3; ++i)nvalues.push_back(f->points[1]->normal(i));
        for(i=0; i<3; ++i)nvalues.push_back(f->points[2]->normal(i));
    }
    // std::cin.get();
    // vertices
    glBindBuffer( GL_ARRAY_BUFFER, vbo[3] );
    glBufferData(GL_ARRAY_BUFFER, pvalues.size()*4, &pvalues[0], GL_STATIC_DRAW );
    // texture
    glBindBuffer( GL_ARRAY_BUFFER, vbo[4] );
    glBufferData(GL_ARRAY_BUFFER, tvalues.size()*4, &tvalues[0], GL_DYNAMIC_DRAW );
    // normals
    glBindBuffer( GL_ARRAY_BUFFER, vbo[5] );
    glBufferData(GL_ARRAY_BUFFER, nvalues.size()*4, &nvalues[0], GL_DYNAMIC_DRAW );

    mMat.setToIdentity();
    mvMat = viewPort.vMat*mMat;
    invTrMat = mvMat.invert();
    invTrMat.transpose();
    // Setup light
    light.currentPos = light.initialLightLoc;
    installLights(b, viewPort.vMat);

    // Copy matrices into uniform variables
    glUniformMatrix4fv( viewPort.mvLoc, 1, GL_FALSE, mvMat.mat.data() );
    glUniformMatrix4fv( viewPort.prLoc, 1, GL_FALSE, viewPort.pMat.mat.data() );
    glUniformMatrix4fv( nLoc, 1, GL_FALSE, invTrMat.mat.data() );
    glUniform1i( typeLoc, 1 );
    //Associate VBO with the corresponding vertex attribute in the vertex shader
    glBindBuffer( GL_ARRAY_BUFFER, vbo[3] );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );
    // Sending texture data
    glBindBuffer( GL_ARRAY_BUFFER, vbo[4] );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 1 );
    // Sending normals
    glBindBuffer( GL_ARRAY_BUFFER, vbo[5] );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 2 );
    // Adjust settings
    // glActiveTexture( GL_TEXTURE0 );
    // glBindTexture( GL_TEXTURE_2D, textureId );

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glDrawArrays( GL_TRIANGLES, 0, faces.size()*3 );
}

void SoftmatsView::setCamera(float az, float ev){
    float r = 20.0f;
    this->camera = { r*sin(az)*cos(ev), r*sin(az)*sin(ev), r*cos(az) };
}

bool SoftmatsView::shouldClose(){
    return glfwWindowShouldClose( window );
}

void SoftmatsView::postDisplay(){
    glfwSwapBuffers( window );
    glfwPollEvents();
}

SoftmatsView::SoftmatsView(){
    init();
}


SoftmatsView::~SoftmatsView(){

}
