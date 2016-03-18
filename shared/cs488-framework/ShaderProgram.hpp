/*
 * ShaderProgram
 */

#pragma once

#include "OpenGLImport.hpp"

#include <string>


class ShaderProgram {
public:
    ShaderProgram();

    virtual ~ShaderProgram();

    void generateProgramObject();

    void attachVertexShader(const char * filePath);

    void attachFragmentShader(const char * filePath);

    void attachGeometryShader(const char * filePath);

    void attachComputeShader(const char * filePath);

    virtual void link();

    void enable() const;

    void disable() const;

    void recompileShaders();

    GLuint getProgramObject() const;

    GLint getUniformLocation(const char * uniformName) const;

    GLint getAttribLocation(const char * attributeName) const;


protected:
    struct Shader {
        GLuint shaderObject;
        std::string filePath;

        Shader();
    };

    Shader vertexShader;
    Shader fragmentShader;
    Shader geometryShader;
    Shader computeShader;

    GLuint programObject;
    GLuint prevProgramObject;
    GLuint activeProgram;

    void extractSourceCode(std::string & shaderSource, const std::string & filePath);

    void extractSourceCodeAndCompile(const Shader &shader);

    GLuint createShader(GLenum shaderType);

    void compileShader(GLuint shaderObject, const std::string & shader, const std::string & filePath);

    void checkCompilationStatus(GLuint shaderObject, const std::string & filePath);

    void attachShaders();

    void checkLinkStatus();

    void deleteShaders();
};

