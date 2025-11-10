#pragma once

#include "Module.h"
#include "GameObject.h"

class Scene : public Module {
public:

	Scene::Scene();
	/*Scene::~Scene();*/

	bool Start();
	bool Update(float dt);
	bool CleanUp();

	GameObject* root;

private:


};
