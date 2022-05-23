#pragma once
/*
   Shader class
   - loading Shader source code, Shader Program creation
*/

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
   public:
      GLuint program;

      Shader(const GLchar* vertPath, const GLchar* fragPath, std::initializer_list<GLchar*> utilPaths = {}, GLuint glMajor = 4, GLuint glMinor = 1) : glMajorVersion(glMajor), glMinorVersion(glMinor)
      {
         const std::string vertSource = loadSource(vertPath);
         const std::string fragSource = loadSource(fragPath);

         std::string utilsSource;
         
         for( auto utilPath : utilPaths )
         {
            utilsSource += loadSource(utilPath) + "\n";
         }

         GLuint vertexShader = compileShader(vertSource, GL_VERTEX_SHADER, utilsSource);
         GLuint fragmentShader = compileShader(fragSource, GL_FRAGMENT_SHADER, utilsSource);

         // Link shaders
         program = glCreateProgram();
         /*
         for( auto utilPath : utilPaths )
         {
            const std::string utilSource = loadSource(utilPath);
            GLuint utilShader = compileShader(utilSource, GL_VERTEX_SHADER);
            glAttachShader(program, utilShader);
         }*/

         glAttachShader(program, vertexShader);
         glAttachShader(program, fragmentShader);
         glLinkProgram(program);
         checkLinkingErrors();

         glDeleteShader(vertexShader);
         glDeleteShader(fragmentShader);
      }

      void use() const noexcept { glUseProgram(program); }
      void del()                { glDeleteProgram(program); }

      #pragma region utility_uniform_functions
         void setBool (const std::string &name, bool value)                             const { glUniform1i (glGetUniformLocation(program, name.c_str()), (int)value); }
         void setInt  (const std::string &name, int value)                               const { glUniform1i (glGetUniformLocation(program, name.c_str()), value); }
         void setUint (const std::string &name, unsigned int value)                             const { glUniform1ui(glGetUniformLocation(program, name.c_str()), value); }
         void setFloat(const std::string &name, float value)                           const { glUniform1f (glGetUniformLocation(program, name.c_str()), value); }

         void setVec2(const std::string &name, const GLfloat value [])                 const { glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]); }
         void setVec2(const std::string &name, const glm::vec2 &value)                 const { glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value)); }
         void setVec2(const std::string &name, float x, float y)                       const { glUniform2f (glGetUniformLocation(program, name.c_str()), x, y); }
         
         void setVec3(const std::string &name, const GLfloat value [])                 const { glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]); }
         void setVec3(const std::string &name, const glm::vec3 &value)                 const { glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value)); }
         void setVec3(const std::string &name, float x, float y, float z)              const { glUniform3f (glGetUniformLocation(program, name.c_str()), x, y, z); }
         
         void setVec4(const std::string &name, const GLfloat value [])                 const { glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]); }
         void setVec4(const std::string &name, const glm::vec4 &value)                 const { glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value)); }
         void setVec4(const std::string &name, float x, float y, float z, float w)           { glUniform4f (glGetUniformLocation(program, name.c_str()), x, y, z, w); }
         
         void setMat2(const std::string &name, const glm::mat2 &mat)                   const { glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat)); }
         
         void setMat3(const std::string &name, const glm::mat3 &mat)                   const { glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat)); }
         
         void setMat4(const std::string &name, const glm::mat4 &mat)                   const { glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat)); }
      #pragma endregion 

   private:
      GLuint glMajorVersion;
      GLuint glMinorVersion;

      const std::string loadSource(const GLchar* sourcePath) const noexcept
      {
         std::string         sourceCode;
         std::ifstream       sourceFile;
         std::stringstream sourceStream;

         sourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

         try
         {
            sourceFile.open(sourcePath); 
            sourceStream << sourceFile.rdbuf();
            sourceFile.close();
            sourceCode = sourceStream.str();
         }
         catch(const std::exception& e)
         {
            std::cerr << e.what() << '\n';
         }

         return sourceCode;
      }
      
      GLuint compileShader(const std::string& shaderSource, GLenum shaderType, const std::string& utilsSource = "") const noexcept
      {
         std::string mergedSource = "";
         // Prepending version
         mergedSource += "#version "+ std::to_string(glMajorVersion) + std::to_string(glMinorVersion) +"0 core\n";

         // Prepending utils
         mergedSource += utilsSource + shaderSource;

         const std::string finalSource = mergedSource;

         std::cout << finalSource << std::endl;

         // Shader creation
         GLuint shader = glCreateShader(shaderType);
         const GLchar* c = finalSource.c_str();
         glShaderSource(shader, 1, &c, NULL);
         glCompileShader(shader);
         checkCompileErrors(shader);
         return shader;
      }

      void checkCompileErrors(GLint shader) const noexcept
      {
         // Check for compile time errors TODO
         GLint success; GLchar infoLog[512];
         glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
         if (!success)
         {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
         }
      }

      void checkLinkingErrors() const noexcept
      {
         GLint success; GLchar infoLog[512];
         glGetProgramiv(program, GL_LINK_STATUS, &success);
         if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
         }
      }
};