
// This code is in the public domain
// Miguel Rodriguez Gallego

#include "AssimpMesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <gtc/type_ptr.hpp>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace udit
{
    AssimpMesh::AssimpMesh() : textureID(0)
    {

    }
    AssimpMesh::~AssimpMesh()
    {
        // Deleting name 0 is a no-op, so this is safe even if load() failed
        glDeleteVertexArrays(1, &vao_id);
        glDeleteBuffers(VBO_COUNT, vbo_ids);

        if (textureID) glDeleteTextures(1, &textureID);
    }

    void AssimpMesh::load(const std::string& mesh_file_path)
    {
        Assimp::Importer importer;

        // 1) Import the model
        //    - PreTransformVertices bakes the node hierarchy into the vertices, so every
        //      mesh of the file ends up in the right place
        //    - GenSmoothNormals only creates normals for meshes that don't have them
        const aiScene* scene = importer.ReadFile
        (
            mesh_file_path,
            aiProcess_Triangulate           |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType           |
            aiProcess_PreTransformVertices  |
            aiProcess_GenSmoothNormals
        );

        // A null scene means the file could not be loaded:
        if (!scene || scene->mNumMeshes == 0)
        {
            throw std::runtime_error
            (
                std::string("Could not load mesh: ") + importer.GetErrorString()
            );
        }

        // 2) Merge every triangle mesh of the file into one set of buffers, so the
        //    whole model is drawn with a single call
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<GLuint>    indices;      // 32-bit, so meshes can have more than 65,535 vertices

        for (unsigned m = 0; m < scene->mNumMeshes; ++m)
        {
            const aiMesh * mesh = scene->mMeshes[m];

            // SortByPType splits points and lines into their own meshes; only triangles are drawn
            if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)) continue;

            const GLuint base_vertex = GLuint(positions.size());

            for (unsigned i = 0; i < mesh->mNumVertices; ++i)
            {
                const aiVector3D & p = mesh->mVertices[i];
                positions.emplace_back(p.x, p.y, p.z);

                if (mesh->HasNormals())
                {
                    const aiVector3D & n = mesh->mNormals[i];
                    normals.emplace_back(n.x, n.y, n.z);
                }
                else normals.emplace_back(0.f, 0.f, 0.f);

                if (mesh->HasTextureCoords(0))
                {
                    // Model files put v = 0 at the bottom of the image, but textures are
                    // uploaded top row first (v = 0 is the top), so v is flipped here:
                    const aiVector3D & t = mesh->mTextureCoords[0][i];
                    uvs.emplace_back(t.x, 1.f - t.y);
                }
                else uvs.emplace_back(0.f, 0.f);
            }

            // Assimp stores indices per face; they are offset by the vertices of the
            // meshes already merged:
            for (unsigned i = 0; i < mesh->mNumFaces; ++i)
            {
                const aiFace & face = mesh->mFaces[i];

                if (face.mNumIndices != 3) continue;

                indices.push_back(base_vertex + face.mIndices[0]);
                indices.push_back(base_vertex + face.mIndices[1]);
                indices.push_back(base_vertex + face.mIndices[2]);
            }
        }

        if (indices.empty())
        {
            throw std::runtime_error("Could not load mesh: " + mesh_file_path + " has no triangles");
        }

        number_of_indices = GLsizei(indices.size());

        // 3) Upload the geometry
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(VBO_COUNT, vbo_ids);

        glBindVertexArray(vao_id);

        // --- Coordinates ---
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COORD_VBO]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // --- Normals --- (used for lighting)
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMAL_VBO]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // --- UVs --- (texture coordinates)
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[UV_VBO]);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // --- Indices ---
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void AssimpMesh::draw(const glm::mat4& modelMatrix, GLuint shaderProgram) const
    {
        // 1) Model uniform
        GLint loc = glGetUniformLocation(shaderProgram, "model_view_matrix");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // 2) Bind the texture if there is one
        if (textureID) 
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            GLint s = glGetUniformLocation(shaderProgram, "sampler");
            glUniform1i(s, 0);
        }

        // 3) Draw the mesh
        glBindVertexArray(vao_id);
        glDrawElements(GL_TRIANGLES, number_of_indices, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}