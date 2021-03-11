#include "softmatsview.h"
#include "../util/openglutils.h"
#include "../util/config.h"

using namespace morph::softmats;

void SoftmatsView::setupGround( Body *ground){
	std::vector<Face *>& faces = ground->getMesh()->getFaces();
	std::vector<Point *>& vert = ground->getMesh()->getVertices();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	int i;

	for( Face* f : faces ){
		for(i=0; i<3; ++i)pvalues.push_back(f->points[0]->x(i));
		for(i=0; i<3; ++i)pvalues.push_back(f->points[1]->x(i));
		for(i=0; i<3; ++i)pvalues.push_back(f->points[2]->x(i));
		
		tvalues.push_back(f->points[0]->uv.s);
		tvalues.push_back(f->points[0]->uv.t);
		tvalues.push_back(f->points[1]->uv.s);
		tvalues.push_back(f->points[1]->uv.t);
		tvalues.push_back(f->points[2]->uv.s);
		tvalues.push_back(f->points[2]->uv.t);
		
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
	glewExperimental = true;
	
	if( !glfwInit() ){ exit(EXIT_FAILURE); }

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	window = glfwCreateWindow( 600, 600, "Soft body simulator", NULL, NULL );
	glfwMakeContextCurrent( window );
	std::cout << "Window created\n";

	if( glewInit() != GLEW_OK ){ exit(EXIT_FAILURE); }

	std::cout << "Glew initialized\n";
	glfwSwapInterval( 1 );
	std::string vshader = Config::getConfig()->getShaderLocation() + "softmats.vsh";
	std::string fshader = Config::getConfig()->getShaderLocation() + "softmats.fsh";
	renderingProgram = OpenglUtils::createShaderProgram(vshader.c_str(), fshader.c_str());
	OpenglUtils::checkOpenGLError();
	std::cout << "Shaders loaded\n";
	camera.x = 0.0f; camera.y = -0.5f; camera.z = 10.5f;
	viewPort.x = 0.0f; viewPort.y = -2.0f; viewPort.z = 0.0f;
	light.initialLightLoc = glm::vec3(5.0f, 2.0f, 2.0f);
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

void SoftmatsView::installLights( Body *b, glm::mat4 vMatrix ){
	float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	light.posV = glm::vec3(vMatrix*glm::vec4(light.currentPos,1.0));
	light.pos[0] = light.posV.x;
	light.pos[1] = light.posV.y;
	light.pos[2] = light.posV.z;

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
	glProgramUniform4fv( renderingProgram, light.posLoc, 1, light.pos );

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
	viewPort.pMat = glm::perspective( 1.0472f, viewPort.aspect, 0.1f, 1000.0f );
	// View and model matrices
	viewPort.vMat = glm::translate( glm::mat4(1.0f), glm::vec3(-camera.x, -camera.y, -camera.z) );
}

void SoftmatsView::displayGround(){
	mMat = glm::mat4(1.0f);
    mvMat = viewPort.vMat*mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	// Setup light
	light.currentPos = glm::vec3(light.initialLightLoc.x, light.initialLightLoc.y, light.initialLightLoc.z);
	installLights(NULL, viewPort.vMat);

	 // Copy matrices into uniform variables
    glUniformMatrix4fv( viewPort.mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat) );
    glUniformMatrix4fv( viewPort.prLoc, 1, GL_FALSE, glm::value_ptr(viewPort.pMat) );
	glUniformMatrix4fv( nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
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
	std::vector<Point *>& vert = b->getMesh()->getVertices();

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
		
		tvalues.push_back(f->points[0]->uv.s);
		tvalues.push_back(f->points[0]->uv.t);
		tvalues.push_back(f->points[1]->uv.s);
		tvalues.push_back(f->points[1]->uv.t);
		tvalues.push_back(f->points[2]->uv.s);
		tvalues.push_back(f->points[2]->uv.t);

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

    mMat = glm::mat4(1.0f);
    mvMat = viewPort.vMat*mMat;
    invTrMat = glm::transpose(glm::inverse(mvMat));
	// Setup light
	light.currentPos = glm::vec3(light.initialLightLoc.x, light.initialLightLoc.y, light.initialLightLoc.z);
	installLights(b, viewPort.vMat);

    // Copy matrices into uniform variables
    glUniformMatrix4fv( viewPort.mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat) );
    glUniformMatrix4fv( viewPort.prLoc, 1, GL_FALSE, glm::value_ptr(viewPort.pMat) );
	glUniformMatrix4fv( nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
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
	this->camera.x = r*sin(az)*cos(ev);
	this->camera.y = r*sin(az)*sin(ev);
	this->camera.z = r*cos(az);
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
