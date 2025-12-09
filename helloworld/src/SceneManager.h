#pragma once
#include "Module.h"
#include "Scene.h"
#include "GameObject.h"
#include <list>
#include <string>
#include <memory>

class SceneManager : public Module {
public:
	SceneManager();
	~SceneManager();

	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;

	//void CreateScene(const std::string& name);

	////for registering existing scene objects
	//void AddScene(std::shared_ptr<Scene> scene);
	//void RemoveScene(const std::string& name);
	void SetActiveScene(const std::string& name);

	std::shared_ptr<GameObject> CreateGameObject(const std::string& name = "GameObject");
	std::shared_ptr<GameObject> CreateEmptyGameObject(const std::string& name, std::shared_ptr<GameObject> parent);

	void DestroyGameObject(std::shared_ptr<GameObject> go);
	std::shared_ptr<GameObject> CreateCube();
	

	void LoadDefaultScene();

	void SaveActiveScene();
	std::shared_ptr<Scene> GetActiveScene() const;
	

private:
	std::shared_ptr<Scene> currentScene = nullptr;
	std::vector<std::shared_ptr<Scene>> scenes;
};
