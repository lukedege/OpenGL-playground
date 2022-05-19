#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <iostream>

#include <utils/mesh.h>

// Model class purpose:
// 1. Open file from disk
// 2. Load data with assimp
// 3. Pass all nodes to data structure
// 4. Create a mesh from data structure (which will setup VBO)

class Model
{
   public:
      std::vector<Mesh> meshes;

      // To enforce the RAII, we need to avoid copy operations
      // by explicitly removing them (they're created by default)
      Model(const Model& copy) = delete;
      Model& operator=(const Model& copy) = delete;

      // Then we have to define the move semantics which replaces
      // the copy ones we just deleted
      Model(Model&& move) = default;
      Model& operator=(Model&& move) noexcept = default;

      Model(const std::string& path) { loadModel(path); }

      void draw() const
      {
         for (size_t i = 0; i < meshes.size(); i++) { meshes[i].draw(); }
      }

   private:
      void loadModel(const std::string& path)
      {
         Assimp::Importer importer;
         
         // Applying various mesh processing functions to the import by assimp
         const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | 
                        aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | 
                        aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

         if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
         {
            // If scene failed to complete or there are some error flags from assimp, then stop application
            std::cout << "ASSIMP ERROR! " << importer.GetErrorString() << std::endl;
            return;
         }
         
         // process the scene tree starting from root node down to its descendants
         processNode(scene->mRootNode, scene);
      }   

      void processNode(const aiNode* node, const aiScene* scene)
      {
         // Process each node
         for (size_t i = 0; i < node->mNumMeshes; i++)
         {
            // each node has an index reference to its mesh, which is contained in scene's mMeshes array
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 

            meshes.emplace_back(processMesh(mesh));
         }

         // recurse through the nodes children 
         for (size_t i = 0; i < node->mNumChildren; i++)
         {
            processNode(node->mChildren[i], scene);
         }
         
      }

      Mesh processMesh(const aiMesh* mesh)
      {
         std::vector<Vertex> vertices;
         std::vector<GLuint>  indices;

         for (size_t i = 0; i < mesh->mNumVertices; i++)
         {
            Vertex vertex;
            glm::vec3 vec3;
            vec3.x = mesh->mVertices[i].x;
            vec3.y = mesh->mVertices[i].y;
            vec3.z = mesh->mVertices[i].z;
            vertex.position = vec3;

            vec3.x = mesh->mNormals[i].x;
            vec3.y = mesh->mNormals[i].y;
            vec3.z = mesh->mNormals[i].z;
            vertex.normal = vec3;

            if(mesh->mTextureCoords[0])
            {
               // The model has UV textures so we assign them
               glm::vec2 vec2;
               vec2.x = mesh->mTextureCoords[0][i].x;
               vec2.y = mesh->mTextureCoords[0][i].y;
               vertex.texCoords = vec2;

               // Since there are UVs, then assimp calculated the tan + bitan
               vec3.x = mesh->mTangents[i].x;
               vec3.y = mesh->mTangents[i].y;
               vec3.z = mesh->mTangents[i].z;
               vertex.tangent = vec3;

               vec3.x = mesh->mBitangents[i].x;
               vec3.y = mesh->mBitangents[i].y;
               vec3.z = mesh->mBitangents[i].z;
               vertex.bitangent = vec3;
            }
            else
            {
               // The model has no UV textures
               vertex.texCoords = glm::vec2(0.f, 0.f);
               std::cout << "Warning: UV not present" << std::endl;
            }

            vertices.emplace_back(vertex);
         }

         for (size_t i = 0; i < mesh->mNumFaces; i++)
         {
            aiFace face = mesh->mFaces[i];

            for (size_t j = 0; j < face.mNumIndices; j++)
            {
               indices.emplace_back(face.mIndices[j]);
            }
         }

         return Mesh{vertices, indices};
      }

};