#include "GameObject.h"

void GameObject::Update()
{
	for (int i; i < Components.size(); i++) {
		Components[i].Update();
	}
}
