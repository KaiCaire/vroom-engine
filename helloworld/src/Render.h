#pragma once
#include "Module.h"
//#include "Shader.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <memory>

class GameObject;
class Shader;

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

    // 3D Rendering
    void RenderFrame(Shader& shader);
    void DrawActiveScene(Shader& shader);
    void DrawGameObject(std::shared_ptr<GameObject> go, Shader& shader);
    void DrawGrid();

    // Shader utilities
    void UpdateShaderMatrices(Shader& shader);

    // Background color
    void SetBackgroundColor(SDL_Color color);

private:
    SDL_Renderer* renderer = nullptr;
    SDL_Rect viewport;
    SDL_Color background;

    // Matrices (updated from Camera each frame)
    glm::mat4 viewMat;
    glm::mat4 projectionMat;

    bool vsync;
};