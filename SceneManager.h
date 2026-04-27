///////////////////////////////////////////////////////////////////////////////
// SceneManager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include "ViewManager.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

class SceneManager
{
public:
    SceneManager(ShaderManager* pShaderManager);
    ~SceneManager();

    struct TEXTURE_INFO {
        std::string tag;
        uint32_t ID;
    };

    struct OBJECT_MATERIAL {
        float ambientStrength;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
        std::string tag;
    };

    // Pass the camera so RenderScene can access view data
    void SetCamera(Camera* pCamera) { m_camera = pCamera; }

    void PrepareScene();
    void RenderScene();

private:
    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;
    Camera* m_camera;

    int m_loadedTextures;
    TEXTURE_INFO m_textureIDs[16];
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    bool CreateGLTexture(const char* filename, std::string tag);
    int FindTextureSlot(std::string tag);

    void SetTransformations(glm::vec3 scaleXYZ, float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees, glm::vec3 positionXYZ);
    void SetShaderColor(float r, float g, float b, float a);
    void SetShaderTexture(std::string textureTag);
    void SetTextureUVScale(float u, float v);

    // Sets per-object material properties for Phong lighting
    void SetShaderMaterial(float ambientStr, float specularStr, float shininess);
};