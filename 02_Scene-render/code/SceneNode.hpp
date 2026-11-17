// This code is in the public domain
// Miguel Rodriguez Gallego

#pragma once

#include <vector>
#include <memory>
#include <glm.hpp>                          // vec3, mat4
#include <gtc/type_ptr.hpp>                // value_ptr
#include <glad/glad.h>                     // GLuint
#include "Mesh.hpp"                         // Full Mesh definition

namespace udit
{
    class SceneNode
    {
    public:
        glm::mat4 localTransform = glm::mat4(1.0f);
        std::vector<std::shared_ptr<Mesh>> meshes;         // Meshes in this node
        std::vector<std::unique_ptr<SceneNode>> children;  // Children in the hierarchy

        // Adds a mesh to this node
        void addMesh(const std::shared_ptr<Mesh>& m)
        {
            meshes.push_back(m);
        }

        // Adds a child node
        void addChild(std::unique_ptr<SceneNode> child)
        {
            children.push_back(std::move(child));
        }

        // Recursive traversal and drawing
        void draw(const glm::mat4& parentTransform, GLuint shaderProgram)
        {
            // 1) Compute the global transform
            glm::mat4 globalTransform = parentTransform * localTransform;

            // 2) Upload the shader uniforms
            GLint mvLoc = glGetUniformLocation(shaderProgram, "model_view_matrix");
            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(globalTransform));

            glm::mat4 normalMat = glm::transpose(glm::inverse(globalTransform));
            GLint nmLoc = glGetUniformLocation(shaderProgram, "normal_matrix");
            glUniformMatrix4fv(nmLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

            // 3) Draw every mesh
            for (auto& m : meshes)
            {
                m->draw(globalTransform, shaderProgram);
            }

            // 4) Draw the children
            for (auto& child : children)
            {
                child->draw(globalTransform, shaderProgram);
            }
        }
    };

}
