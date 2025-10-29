#pragma once
#include "Component.h"
#include "Mesh.h"  // Tu clase Mesh actual

class Shader;  // Forward declaration

class RenderMeshComponent : public Component {
public:
    RenderMeshComponent(GameObject* owner);
    ~RenderMeshComponent() override;

    // Component interface
    void Enable() override;
    void Update() override;
    void Disable() override;
    void OnEditor() override;

    // Mesh management
    void SetMesh(Mesh* newMesh);
    Mesh* GetMesh() const { return mesh; }

    // Rendering
    void Render(Shader* shader);  // Ahora recibe el shader como parámetro

    // Drawing properties
    bool GetCastShadows() const { return castShadows; }
    void SetCastShadows(bool cast) { castShadows = cast; }

    bool GetReceiveShadows() const { return receiveShadows; }
    void SetReceiveShadows(bool receive) { receiveShadows = receive; }

private:
    Mesh* mesh;  // Pointer to mesh data (not owned by this component)
    bool castShadows;
    bool receiveShadows;
};