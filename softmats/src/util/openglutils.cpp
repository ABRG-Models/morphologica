#include "openglutils.h"
// Necessary for stb_image to work
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace morph::softmats;

string OpenglUtils::readShaderSource( const char* path ){
    string content;
    ifstream fileStream( path, ios::in );
    string line = "";

    while( !fileStream.eof() ){
        getline( fileStream, line );
        content.append( line + "\n" );
    }


        fileStream.close();
        return content;
}

void OpenglUtils::printShaderLog( GLuint shader ){
    int len = 0;
    int chWrittn = 0;
    char *log;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &len );

    if( len > 0 ){
        log = (char *)malloc( len );
        glGetShaderInfoLog( shader, len, &chWrittn, log );
        cout << "Shader Info Log: " << log << endl;
        free( log );
    }
}

void OpenglUtils::printProgramLog( int prog ){
    int len = 0;
    int chWrittn = 0;
    char *log;
    glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &len );

    if( len > 0 ){
        log = (char *)malloc(len);
        glGetProgramInfoLog( prog, len, &chWrittn, log );
        cout << "Program Info Log: " << log << endl;
        free( log );
    }
}

GLuint OpenglUtils::loadTextureChecker( int width, int height ){
    unsigned char data[width][height][3];
    unsigned int i, j, k;
    int v = 128;

    for( i = 0; i < width; ++i)
        for( j = 0; j < height; ++j ){
            v = (((i & 0x80) == 0) ^ ((j & 0x80) == 0)) * 255;

            for( k = 0; k < 3; ++k )
                data[i][j][k] = v;
        }
    
    return loadTexture( data, width, height );
}

GLuint OpenglUtils::loadTextureImage( const char* textImagePath ){
    int width, height, nrChannels;
    unsigned char *data = stbi_load( textImagePath, &width, &height, &nrChannels, 0 );
    return loadTexture( data, width, height );
}

GLuint OpenglUtils::loadTexture( const void* data, int width, int height ){       

    if( data != NULL ){
        GLuint texture;
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glGenerateMipmap(GL_TEXTURE_2D);
        OpenglUtils::checkOpenGLError();
        std::cout << "Texture loaded succesfully: " << texture << "\n";
        return texture;
    }else{
        std::cerr << "Error loading the texture!";
        return 0;
    }
}

bool OpenglUtils::checkOpenGLError(){
    bool foundError = false;
    int glErr = glGetError();

    while( glErr != GL_NO_ERROR ){
        cout << "glError: " << glErr << endl;
        foundError = true;
        glErr = glGetError();
    }

    return foundError;
}

GLuint OpenglUtils::createShaderProgram( const char* vn, const char* fn ){
    GLint vCompiled, fCompiled, linked;
    string vs = readShaderSource( vn );
    string fs = readShaderSource( fn );
    const char *vshaderSource = vs.c_str();
    const char *fshaderSource = fs.c_str();
    
    GLuint vShader = glCreateShader( GL_VERTEX_SHADER );
    GLuint fShader = glCreateShader( GL_FRAGMENT_SHADER );
    
    glShaderSource( vShader, 1, &vshaderSource, NULL );
    glShaderSource( fShader, 1, &fshaderSource, NULL );
    glCompileShader( vShader );
    glGetShaderiv( vShader, GL_COMPILE_STATUS, &vCompiled );

    if( checkOpenGLError() || vCompiled != 1 ){
        printShaderLog( vShader );
    }

    glCompileShader( fShader );
    glGetShaderiv( vShader, GL_COMPILE_STATUS, &fCompiled );

    if( checkOpenGLError() || fCompiled != 1 ){
        printShaderLog( fShader );
    }

    GLuint vfProgram = glCreateProgram();
    glAttachShader( vfProgram, vShader );
    glAttachShader( vfProgram, fShader );
    glLinkProgram( vfProgram );
    glGetProgramiv( vfProgram, GL_LINK_STATUS, &linked );

    if( checkOpenGLError() || linked != 1 ){
        printProgramLog( vfProgram );
    }

    return vfProgram;
}