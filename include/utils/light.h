#pragma once

#include <utils/shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct LightAttributes
{
   // Light values
   glm::vec3 ambient;
   glm::vec3 diffuse;
   glm::vec3 specular;

   // Decay values
   float constant;
   float linear;
   float quadratic;
};

class Light
{
   public:
      LightAttributes attrs;

      Light(LightAttributes &attrs) : attrs(attrs) {}

      virtual void setLightAttrs(Shader shader, std::string &prefix)
      {
         shader.setVec3(prefix + "diffuseColor", attrs.diffuse);
         shader.setVec3(prefix + "specularColor", attrs.specular);
         shader.setVec3(prefix + "ambientColor", attrs.ambient);

         shader.setFloat(prefix + "constant", attrs.constant);
         shader.setFloat(prefix + "linear", attrs.linear);
         shader.setFloat(prefix + "quadratic", attrs.quadratic);
      }
      virtual void setup(Shader shader, size_t index) = 0;
};

class PointLight : Light
{
   public:
      glm::vec3 position;

      PointLight(glm::vec3 position, LightAttributes &attrs) :
         Light(attrs), position(position) {}

      void setup(Shader shader, size_t index) override
      {
         std::string prefix = "pointLights[" + std::to_string(index) + "].";
         setLightAttrs(shader, prefix);
         shader.setVec3(prefix + "position", position);
      }
};

class DirectionalLight : Light
{
   public:
      glm::vec3 direction;

      DirectionalLight(glm::vec3 direction, LightAttributes &attrs) :
         Light(attrs), direction(direction) {}

      void setup(Shader shader, size_t index) override
      {
         std::string prefix = "directionalLights[" + std::to_string(index) + "].";
         setLightAttrs(shader, prefix);
         shader.setVec3(prefix + "direction", direction);
      }
};

class SpotLight : Light
{
   public:
      glm::vec3 position;
      glm::vec3 direction;
      float cutoffAngle;
   
      SpotLight(glm::vec3 position, glm::vec3 direction, float cutoffAngle, LightAttributes &attrs) :
         Light(attrs), position(position), direction(direction), cutoffAngle(cutoffAngle) {}

      void setup(Shader shader, size_t index) override
      {
         std::string prefix = "spotLights[" + std::to_string(index) + "].";
         setLightAttrs(shader, prefix);
         shader.setVec3(prefix + "position", position);
         shader.setVec3(prefix + "direction", direction);
         shader.setFloat(prefix + "cutoffAngle", cutoffAngle);
      }
};