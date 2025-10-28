#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderMeshComponent.h"
#include "MaterialComponent.h"
#include <algorithm>


void GameObject::Update()
{
	for (int i; i < Components.size(); i++) {
		Components[i].Update();
	}
}
