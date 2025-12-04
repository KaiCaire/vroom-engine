#pragma once
#include "Component.h"
#include "ResourceMesh.h"  
#include "MaterialComponent.h"

class Shader;  // Forward declaration


class RenderMeshComponent : public Component {
public:
    RenderMeshComponent(std::shared_ptr<GameObject> owner);
    ~RenderMeshComponent() override;

    // Component interface
    void Enable() override;
    void Update() override;
    void Disable() override;
    void OnEditor() override;

    // Mesh management
    void SetMesh(std::shared_ptr<ResourceMesh> newMesh);
    std::shared_ptr<ResourceMesh>  GetMesh() const { return mesh; }
    ResourceMesh*  GetMeshPointer() const { return mesh.get(); }


    VroomUUID GetMeshUUID() { return meshUUID; }
    void SetMeshUUID(VroomUUID id) {
        meshUUID = id;
        if(mesh) mesh->SetUUID(id);
    }

    void Render(Shader* shader);  

private:
    std::shared_ptr<ResourceMesh> mesh;  // Pointer to mesh data (not owned by this component)
    VroomUUID meshUUID = 0;
    //bool drawFaceNormals;
    //bool drawVertNormals;
};