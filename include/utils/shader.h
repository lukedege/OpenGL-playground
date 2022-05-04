#pragma once
/*
   Shader class
   - loading Shader source code, Shader Program creation
*/

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
   public:
      GLuint program;

      Shader(const GLchar* vertPath, const GLchar* fragPath)
      {
         const GLchar* vertSource = loadSource(vertPath);
         const GLchar* fragSource = loadSource(fragPath);

         GLuint vertexShader = compileShader(vertSource, GL_VERTEX_SHADER);
         GLuint fragmentShader = compileShader(fragSource, GL_FRAGMENT_SHADER);

         // Link shaders
         program = glCreateProgram();
         glAttachShader(program, vertexShader);
         glAttachShader(program, fragmentShader);
         glLinkProgram(program);
         checkLinkingErrors();

         glDeleteShader(vertexShader);
         glDeleteShader(fragmentShader);
      }


      void use() const noexcept { glUseProgram(program); }
      void del()                { glDeleteProgram(program); }

   private:
      const GLchar* loadSource(const GLchar* sourcePath) const noexcept
      {
         std::string         sourceCode;
         std::ifstream       sourceFile;
         std::stringstream sourceStream;

         sourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

         try
         {
            sourceFile.open(sourcePath); 
            sourceStream << sourceFile.rdbuf();
            //stream.close();
            sourceCode = sourceStream.str();
            //std::cout << sourceCode;
         }
         catch(const std::exception& e)
         {
            std::cerr << e.what() << '\n';
         }

         return sourceCode.c_str();
      }
      
      GLuint compileShader(const GLchar* shaderSource, GLenum shaderType) const noexcept
      {
         GLuint shader = glCreateShader(shaderType);
         glShaderSource(shader, 1, &shaderSource, NULL);
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