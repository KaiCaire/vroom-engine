#include "Component.h"
#include "GameObject.h"

Component::Component(GameObject* owner, ComponentType type)
    : owner(owner), type(type), active(true) {
}

void Component::SetActive(bool isActive) {
    if (active == isActive) return;

    active = isActive;

    if (active) {
        Enable();
    }
    else {
        Disable();
    }
}