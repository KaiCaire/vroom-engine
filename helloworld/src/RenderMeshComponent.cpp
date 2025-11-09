#include "RenderMeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"
#include "GameObject.h"
#include "Shader.h"
#include "OpenGL.h"
#include "Application.h"


RenderMeshComponent::RenderMeshComponent(std::shared_ptr<GameObject> owner)
    : Component(owner, ComponentType::MESH_RENDERER),
    mesh(nullptr) {

   
}

RenderMeshComponent::~RenderMeshComponent() {
    
    mesh = nullptr;
}

void RenderMeshComponent::Enable() {
    // Component enabled
}

void RenderMeshComponent::Update() {
    
}

void RenderMeshComponent::Disable() {
    // Component disabled
}

void RenderMeshComponent::OnEditor() {
    //laracode aqui
}

void RenderMeshComponent::SetMesh(std::shared_ptr<Mesh> newMesh) {
    mesh = newMesh;
}

void RenderMeshComponent::Render(Shader* shader) {
    if (!mesh || !active || !shader) return;

    auto sharedOwner = owner.lock();
    if (!sharedOwner) return;

    auto transform = std::dynamic_pointer_cast<TransformComponent>(sharedOwner->GetComponent(ComponentType::TRANSFORM));
    if (!transform)
        return;

    auto material = std::dynamic_pointer_cast<MaterialComponent>(sharedOwner->GetComponent(ComponentType::MATERIAL));
    if (!material)
        return;

    // Apply transformation matrix
    glm::mat4 modelMatrix = transform->GetGlobalTransform();
    glEnable(GL_DEPTH_TEST);

    

    shader->Use();

    shader->setMat4("model", modelMatrix);
    mesh->Draw(*shader);
    
    
    if (drawOutline) {
        Shader* outlineShader = Application::GetInstance().openGL.get()->singleColorShader;

        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        // First pass: Write to stencil (invisible)
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        shader->Use();
        Application::GetInstance().openGL.get()->SetUpVertShader(shader);
        shader->setMat4("model", modelMatrix);

        mesh->Draw(*shader);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);  
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        outlineShader->Use();
        Application::GetInstance().openGL.get()->SetUpVertShader(outlineShader);
        glm::mat4 scaledMat = glm::scale(modelMatrix, glm::vec3(1.05));
        outlineShader->setMat4("model", scaledMat);
        outlineShader->setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 0.1f));  // Green

        mesh->Draw(*outlineShader);

        
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
    }



    

    
    

}

void RenderMeshComponent::DrawOutline(Shader* shader)
{
    if (glIsEnabled(GL_STENCIL_TEST))
    {

        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);

        // Render the mesh into the stencil buffer.

        glEnable(GL_STENCIL_TEST);

        glStencilFunc(GL_ALWAYS, 1, -1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        mesh->Draw(*shader);

        // Render the thick wireframe version.

        glStencilFunc(GL_NOTEQUAL, 1, -1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);


        /*if (object->GetSelect() == ObjectSelected::SELECTED)
        {
            glColor3f(1.f, 0.7f, 0.f);
            glLineWidth(7);
        }
        else if (object->GetSelect() == ObjectSelected::CHILD_FROM_PARENT_SELECTED)
        {
            glColor3f(0.f, 1.f, 1.f);
            glLineWidth(4);
        }*/
        /*mesh->Draw(*outlineShader);*/


        /*--------------------------------------*/
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisableClientState(GL_VERTEX_ARRAY);
        glLineWidth(1);
    }

}




