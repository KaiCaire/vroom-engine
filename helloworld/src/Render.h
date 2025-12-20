#pragma once
#include "Module.h"
//#include "Shader.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <memory>

class GameObject;
class Shader;
struct AABB;

class Render : public Module {
public:
    Render();
    ~Render();

    bool Awake() override;
    bool Start() override;
    bool PreUpdate() override;
    bool Update(float dt) override;
    bool PostUpdate() override;
    bool CleanUp() override;

    void InitSceneFBO(int w, int h);
    void InitGameFBO(int w, int h);

    void DrawGameView(Shader& shader);

    // 3D Rendering
    void RenderFrame(Shader& shader);
    void DrawActiveScene(Shader& shader);
    void DrawGameObject(std::shared_ptr<GameObject> go, Shader& shader);
    void DrawGrid();

    //aabb drawing
    void DrawAABB(const AABB& bounds, const glm::vec4& color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    void DrawRay(const glm::vec3& origin, const glm::vec3& direction, const glm::vec4& color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    // Shader utilities
    void UpdateShaderMatrices(Shader& shader);

    // Background color
    void SetBackgroundColor(SDL_Color color);

    unsigned int sceneTextureID = 0;
    unsigned int gameTextureID = 0;

    int gameWidth = 1280;
    int gameHeight = 720;

private:
    SDL_Renderer* renderer = nullptr;
    SDL_Rect camera;
    SDL_Rect viewport;
    SDL_Color background;

    // Matrices (updated from Camera each frame)
    glm::mat4 viewMat;
    glm::mat4 projectionMat;

    bool vsync;

    unsigned int sceneFBO = 0;
    unsigned int sceneRBO = 0;

    unsigned int gameFBO = 0;
    unsigned int gameRBO = 0;
};

