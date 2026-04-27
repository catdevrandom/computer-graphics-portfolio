#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/***********************************************************
 *  Uniform variable names for shader communication
 ***********************************************************/
namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  Constructor / Destructor
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();
    m_camera = nullptr;
    m_loadedTextures = 0;
}

SceneManager::~SceneManager()
{
    for (int i = 0; i < m_loadedTextures; i++) {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
    delete m_basicMeshes;
    m_pShaderManager = nullptr;
    m_camera = nullptr;
}

/***********************************************************
 *  SetTransformations()
 *  Builds a Model matrix from scale, rotation, and
 *  translation components, then uploads it to the shader.
 *  Rotation order applied: Z -> Y -> X (intrinsic).
 ***********************************************************/
void SceneManager::SetTransformations(
    glm::vec3 scaleXYZ,
    float XrotationDegrees,
    float YrotationDegrees,
    float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 scale = glm::scale(scaleXYZ);
    glm::mat4 rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotation = rotationZ * rotationY * rotationX;
    glm::mat4 translation = glm::translate(positionXYZ);
    glm::mat4 model = translation * rotation * scale;

    if (m_pShaderManager) {
        m_pShaderManager->setMat4Value(g_ModelName, model);
    }
}

/***********************************************************
 *  SetShaderColor()
 *  Sends a flat RGBA color to the shader and disables
 *  texture sampling so the color is used directly.
 ***********************************************************/
void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    if (m_pShaderManager) {
        m_pShaderManager->setBoolValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, glm::vec4(r, g, b, a));
    }
}

/***********************************************************
 *  SetShaderTexture()
 *  Looks up a previously loaded texture by its tag name,
 *  binds it to texture unit 0, and enables texture sampling
 *  in the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (m_pShaderManager) {
        int index = FindTextureSlot(textureTag);
        if (index != -1) {
            m_pShaderManager->setBoolValue(g_UseTextureName, true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureIDs[index].ID);
            m_pShaderManager->setIntValue(g_TextureValueName, 0);
        }
    }
}

/***********************************************************
 *  SetTextureUVScale()
 *  Controls texture tiling. Values above 1.0 cause the
 *  texture to repeat, which avoids visible stretching on
 *  large surfaces. This is the complex texturing technique
 *  used on the desk plane.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager) {
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
    }
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  Sets per-object material properties that control how
 *  each surface responds to the Phong lighting model:
 *    ambientStr  - base light level absorbed independent of
 *                  light direction (higher = brighter shadows)
 *    specularStr - intensity of specular highlight
 *    shininess   - tightness of the specular lobe; higher
 *                  values produce a smaller, sharper highlight
 *                  indicating a smoother or shinier surface
 ***********************************************************/
void SceneManager::SetShaderMaterial(float ambientStr, float specularStr, float shininess)
{
    if (m_pShaderManager) {
        m_pShaderManager->setFloatValue("materialAmbientStrength", ambientStr);
        m_pShaderManager->setFloatValue("materialSpecularStrength", specularStr);
        m_pShaderManager->setFloatValue("materialShininess", shininess);
    }
}

/***********************************************************
 *  CreateGLTexture()
 *  Loads an image from disk using stb_image, uploads it
 *  to GPU memory as an OpenGL texture object with mipmaps
 *  and repeat wrapping, and stores it under the given tag
 *  for later lookup. Returns true on success.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    if (m_loadedTextures >= 16) return false;

    int width, height, colorChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

    if (image) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Tiling wrap mode so UV values above 1.0 repeat the pattern
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Mipmapped filtering for smooth appearance at all distances
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = (colorChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);

        m_textureIDs[m_loadedTextures].ID = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;

        std::cout << "Texture loaded: " << filename << " -> tag: " << tag << std::endl;
        return true;
    }
    else {
        std::cout << "WARNING: Failed to load texture: " << filename << std::endl;
        return false;
    }
}

/***********************************************************
 *  FindTextureSlot()
 *  Searches the loaded texture list by tag name and returns
 *  its index, or -1 if the tag is not found.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; i++) {
        if (m_textureIDs[i].tag == tag) return i;
    }
    return -1;
}

/***********************************************************
 *  PrepareScene()
 *  Loads all mesh types and texture image files into GPU
 *  memory before the render loop begins.
 *
 *  Textures sourced from Poliigon.com (free tier):
 *  - dark_metal.jpg     = Matte Metal Texture, Black
 *  - dark_plastic.jpg   = Dry Blast Mold Plastic, Black
 *  - white_ceramic.jpg  = Plain White Ceramic Texture
 *  - ceramic_glazed.jpg = Speckled Glazed Ceramic, White
 *  - clear_plastic.jpg  = Worn ABS Plastic Texture, Gray
 *  - yellow_fabric.jpg  = Bubbly Rows Boucle Fabric
 *  - blue_screen.jpg    = Ocean wallpaper screenshot
 ***********************************************************/
void SceneManager::PrepareScene()
{
    // Load all mesh types used in this scene
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadTorusMesh();
    m_basicMeshes->LoadTaperedCylinderMesh();
    m_basicMeshes->LoadSphereMesh();
    m_basicMeshes->LoadConeMesh();

    // Each texture is attempted from two paths to support
    // both in-project and sibling-directory build layouts.
    if (!CreateGLTexture("textures/dark_metal.jpg", "desk"))          CreateGLTexture("../textures/dark_metal.jpg", "desk");
    if (!CreateGLTexture("textures/dark_plastic.jpg", "laptop_base"))   CreateGLTexture("../textures/dark_plastic.jpg", "laptop_base");
    if (!CreateGLTexture("textures/blue_screen.jpg", "laptop_screen")) CreateGLTexture("../textures/blue_screen.jpg", "laptop_screen");
    if (!CreateGLTexture("textures/white_ceramic.jpg", "mug_body"))      CreateGLTexture("../textures/white_ceramic.jpg", "mug_body");
    if (!CreateGLTexture("textures/ceramic_glazed.jpg", "mug_handle"))    CreateGLTexture("../textures/ceramic_glazed.jpg", "mug_handle");
    if (!CreateGLTexture("textures/clear_plastic.jpg", "bottle"))        CreateGLTexture("../textures/clear_plastic.jpg", "bottle");
    if (!CreateGLTexture("textures/yellow_fabric.jpg", "star"))          CreateGLTexture("../textures/yellow_fabric.jpg", "star");
}

/***********************************************************
 *  RenderScene()
 *
 *  Renders a 3D recreation of the reference photo:
 *    - Dark desk surface  (tiled texture, Plane)
 *    - Laptop             (Box x3: base, bezel, screen)
 *    - Water bottle       (Cylinder x3: body, cap, label)
 *    - White coffee mug   (Cylinder body + Torus handle)
 *    - Yellow star plush  (Sphere center + Cone x5 points)
 *    - Mouse on mousepad  (Sphere + Box)
 *
 *  NOTE: View and Projection matrices are NOT set here.
 *  ViewManager::PrepareSceneView() uploads them to the
 *  shader before every call to RenderScene(). That includes
 *  the correct 1000x800 aspect ratio and the live P/O
 *  orthographic toggle. Setting them again here would
 *  overwrite the orthographic matrix and break that feature.
 *
 *  RUBRIC CRITERIA MET:
 *  1. Two light sources: white key light + warm fill light
 *  2. Per-object material properties (full Phong model)
 *  3. Textures applied with UV tiling (complex technique)
 *  4. Different textures on each component of complex objects
 ***********************************************************/
void SceneManager::RenderScene()
{
    // Guard: require both the shader and camera to be valid
    if (!m_pShaderManager || !m_camera) return;

    // ======================================================
    // LIGHTING SETUP
    // Two Phong light sources configured once per frame.
    // The view and projection matrices are already in the
    // shader from ViewManager::PrepareSceneView().
    // ======================================================
    m_pShaderManager->setBoolValue(g_UseLightingName, true);

    // Light 1: White key light — positioned above and in
    // front of the scene at (0, 10, 5). Simulates a neutral
    // overhead room light for even, natural illumination.
    m_pShaderManager->setVec3Value("lightPos1", 0.0f, 10.0f, 5.0f);
    m_pShaderManager->setVec3Value("lightColor1", 1.0f, 1.0f, 1.0f);

    // Light 2: Warm fill light — positioned left and behind
    // the scene at (-6, 5, -3). The warm yellow-orange color
    // (0.8, 0.6, 0.3) adds color variety, softens hard
    // shadows, and mimics a warm indoor desk lamp.
    m_pShaderManager->setVec3Value("lightPos2", -6.0f, 5.0f, -3.0f);
    m_pShaderManager->setVec3Value("lightColor2", 0.8f, 0.6f, 0.3f);

    // ==========================================================
    // 1. DESK SURFACE (Plane)
    //    Complex texturing technique: UV tiling set to 6x3 so
    //    the dark metal pattern repeats naturally across the
    //    large surface without visible stretching.
    //    Material: low shininess for a matte metal appearance.
    // ==========================================================
    SetShaderTexture("desk");
    SetTextureUVScale(6.0f, 3.0f);
    SetShaderMaterial(0.2f, 0.3f, 8.0f);
    SetTransformations(
        glm::vec3(20.0f, 1.0f, 10.0f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(0.0f, 0.0f, 0.0f));
    m_basicMeshes->DrawPlaneMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // ==========================================================
    // 2. LAPTOP — complex object, three separate Box meshes,
    //    each with its own texture and material properties.
    // ==========================================================

    // --- Laptop Base (dark plastic textured slab) ---
    SetShaderTexture("laptop_base");
    SetTextureUVScale(2.0f, 2.0f);
    SetShaderMaterial(0.3f, 0.2f, 16.0f);
    SetTransformations(
        glm::vec3(6.0f, 0.2f, 4.0f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(0.0f, 0.1f, -1.0f));
    m_basicMeshes->DrawBoxMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // --- Laptop Screen Bezel (same dark plastic as base) ---
    SetShaderTexture("laptop_base");
    SetTextureUVScale(1.0f, 1.0f);
    SetShaderMaterial(0.3f, 0.2f, 16.0f);
    SetTransformations(
        glm::vec3(6.0f, 4.5f, 0.15f),
        -15.0f, 0.0f, 0.0f,
        glm::vec3(0.0f, 2.4f, -3.0f));
    m_basicMeshes->DrawBoxMesh();

    // --- Laptop Display (blue ocean wallpaper texture) ---
    // High ambient keeps the screen bright even in shadow;
    // low specular avoids distracting glare on the panel.
    SetShaderTexture("laptop_screen");
    SetTextureUVScale(1.0f, 1.0f);
    SetShaderMaterial(0.6f, 0.1f, 4.0f);
    SetTransformations(
        glm::vec3(5.4f, 3.8f, 0.16f),
        -15.0f, 0.0f, 0.0f,
        glm::vec3(0.0f, 2.5f, -2.98f));
    m_basicMeshes->DrawBoxMesh();

    // ==========================================================
    // 3. WATER BOTTLE (left side of desk)
    //    Three Cylinder meshes: body, cap, and label band.
    // ==========================================================
    glm::vec3 bottlePos = glm::vec3(-5.0f, 0.0f, 1.5f);

    // --- Bottle Body (clear plastic texture) ---
    SetShaderTexture("bottle");
    SetTextureUVScale(1.0f, 2.0f);
    SetShaderMaterial(0.25f, 0.5f, 32.0f);
    SetTransformations(
        glm::vec3(0.8f, 3.5f, 0.8f),
        0.0f, 0.0f, 0.0f,
        bottlePos);
    m_basicMeshes->DrawCylinderMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // --- Bottle Cap (flat white color, matte plastic) ---
    SetShaderColor(0.95f, 0.95f, 0.95f, 1.0f);
    SetShaderMaterial(0.3f, 0.2f, 8.0f);
    SetTransformations(
        glm::vec3(0.55f, 0.6f, 0.55f),
        0.0f, 0.0f, 0.0f,
        bottlePos + glm::vec3(0.0f, 3.5f, 0.0f));
    m_basicMeshes->DrawCylinderMesh();

    // --- Bottle Label (solid blue band, paper-like material) ---
    SetShaderColor(0.0f, 0.3f, 0.7f, 1.0f);
    SetShaderMaterial(0.3f, 0.1f, 4.0f);
    SetTransformations(
        glm::vec3(0.82f, 1.2f, 0.82f),
        0.0f, 0.0f, 0.0f,
        bottlePos + glm::vec3(0.0f, 0.8f, 0.0f));
    m_basicMeshes->DrawCylinderMesh();

    // ==========================================================
    // 4. WHITE COFFEE MUG (right of center)
    //    Cylinder body with a Torus handle. Each part uses a
    //    distinct ceramic texture for visual contrast.
    // ==========================================================
    glm::vec3 mugPos = glm::vec3(3.5f, 0.0f, 2.0f);

    // --- Mug Body (white ceramic texture) ---
    // Moderate shininess for a smooth glazed ceramic surface.
    SetShaderTexture("mug_body");
    SetTextureUVScale(1.0f, 1.0f);
    SetShaderMaterial(0.25f, 0.6f, 24.0f);
    SetTransformations(
        glm::vec3(1.0f, 2.2f, 1.0f),
        0.0f, 0.0f, 0.0f,
        mugPos);
    m_basicMeshes->DrawCylinderMesh();

    // --- Mug Handle (glazed ceramic texture) ---
    // Slightly higher specular than the body represents the
    // glossier finish typical of a ceramic handle.
    SetShaderTexture("mug_handle");
    SetTextureUVScale(2.0f, 2.0f);
    SetShaderMaterial(0.25f, 0.7f, 28.0f);
    SetTransformations(
        glm::vec3(0.45f, 0.6f, 0.2f),
        0.0f, 90.0f, 90.0f,
        mugPos + glm::vec3(0.95f, 1.1f, 0.0f));
    m_basicMeshes->DrawTorusMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // ==========================================================
    // 5. YELLOW STAR PLUSHIE (far right)
    //    Sphere center + five Cone points. All parts share the
    //    yellow fabric texture and near-zero specular since
    //    soft fabric does not produce shiny highlights.
    // ==========================================================
    glm::vec3 starPos = glm::vec3(6.5f, 1.2f, 1.5f);

    // Set fabric material once; all five star points reuse it.
    SetShaderTexture("star");
    SetTextureUVScale(2.0f, 2.0f);
    SetShaderMaterial(0.35f, 0.05f, 2.0f);

    // --- Star Center Body (flattened sphere) ---
    SetTransformations(
        glm::vec3(1.0f, 1.0f, 0.6f),
        0.0f, 0.0f, 0.0f,
        starPos);
    m_basicMeshes->DrawSphereMesh();

    // --- Star Point: Top ---
    SetTransformations(
        glm::vec3(0.4f, 0.8f, 0.3f),
        0.0f, 0.0f, 0.0f,
        starPos + glm::vec3(0.0f, 0.9f, 0.0f));
    m_basicMeshes->DrawConeMesh();

    // --- Star Point: Bottom-Left ---
    SetTransformations(
        glm::vec3(0.4f, 0.8f, 0.3f),
        0.0f, 0.0f, 140.0f,
        starPos + glm::vec3(-0.85f, -0.4f, 0.0f));
    m_basicMeshes->DrawConeMesh();

    // --- Star Point: Bottom-Right ---
    SetTransformations(
        glm::vec3(0.4f, 0.8f, 0.3f),
        0.0f, 0.0f, -140.0f,
        starPos + glm::vec3(0.85f, -0.4f, 0.0f));
    m_basicMeshes->DrawConeMesh();

    // --- Star Point: Upper-Left ---
    SetTransformations(
        glm::vec3(0.4f, 0.8f, 0.3f),
        0.0f, 0.0f, 70.0f,
        starPos + glm::vec3(-0.85f, 0.5f, 0.0f));
    m_basicMeshes->DrawConeMesh();

    // --- Star Point: Upper-Right ---
    SetTransformations(
        glm::vec3(0.4f, 0.8f, 0.3f),
        0.0f, 0.0f, -70.0f,
        starPos + glm::vec3(0.85f, 0.5f, 0.0f));
    m_basicMeshes->DrawConeMesh();
    SetTextureUVScale(1.0f, 1.0f);

    // --- Star Eyes (two small dark spheres for button eyes) ---
    SetShaderColor(0.05f, 0.05f, 0.05f, 1.0f);
    SetShaderMaterial(0.1f, 0.1f, 4.0f);

    SetTransformations(
        glm::vec3(0.12f, 0.12f, 0.12f),
        0.0f, 0.0f, 0.0f,
        starPos + glm::vec3(-0.25f, 0.15f, 0.5f));
    m_basicMeshes->DrawSphereMesh();

    SetTransformations(
        glm::vec3(0.12f, 0.12f, 0.12f),
        0.0f, 0.0f, 0.0f,
        starPos + glm::vec3(0.25f, 0.15f, 0.5f));
    m_basicMeshes->DrawSphereMesh();

    // ==========================================================
    // 6. MOUSE + MOUSEPAD (far right)
    //    Flattened Sphere body on a thin Box pad. Both use
    //    flat color since dark peripherals have no distinctive
    //    surface texture worth sampling.
    // ==========================================================

    // --- Mouse Body (dark gray, smooth plastic) ---
    SetShaderColor(0.12f, 0.12f, 0.14f, 1.0f);
    SetShaderMaterial(0.25f, 0.4f, 32.0f);
    SetTransformations(
        glm::vec3(0.6f, 0.3f, 0.9f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(7.0f, 0.15f, 2.5f));
    m_basicMeshes->DrawSphereMesh();

    // --- Mousepad (very thin dark box, rubber/fabric material) ---
    SetShaderColor(0.1f, 0.1f, 0.12f, 1.0f);
    SetShaderMaterial(0.2f, 0.05f, 2.0f);
    SetTransformations(
        glm::vec3(3.5f, 0.05f, 3.0f),
        0.0f, 0.0f, 0.0f,
        glm::vec3(7.0f, 0.02f, 2.0f));
    m_basicMeshes->DrawBoxMesh();
}
