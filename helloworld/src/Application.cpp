#include "Application.h"
#include <iostream>
#include "Log.h"

#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "OpenGL.h"
#include "FileSystem.h"
#include "Textures.h"
#include "GUIManager.h"



// Constructor
Application::Application() {

    LOG("Constructor Application::Application");

    // Modules
    window = std::make_shared<Window>();
    guiManager = std::make_shared<GUIManager>();
    input = std::make_shared<Input>();
    render = std::make_shared<Render>();
    openGL = std::make_shared<OpenGL>();
    fileSystem = std::make_shared<FileSystem>();
    textures = std::make_shared<Texture>();

    // Ordered for awake / Start / Update
    // Reverse order of CleanUp
    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(guiManager));
    AddModule(std::static_pointer_cast<Module>(input));
    AddModule(std::static_pointer_cast<Module>(textures));


    // Render last 
    AddModule(std::static_pointer_cast<Module>(openGL));
    AddModule(std::static_pointer_cast<Module>(fileSystem));
    AddModule(std::static_pointer_cast<Module>(render));

}

// Static method to get the instance of the Application class, following the singletn pattern
Application& Application::GetInstance() {
    static Application instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

void Application::AddModule(std::shared_ptr<Module> module) {
    module->Init();
    moduleList.push_back(module);
}

// Called before render is available
bool Application::Awake() {

    LOG("Application::Awake");

    //Iterates the module list and calls Awake on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Awake();
        if (!result) {
            break;
        }
    }

    return result;
}

// Called before the first frame
bool Application::Start() {
    LOG("Application::Start");

    //Iterates the module list and calls Start on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Start();
        if (!result) {
            break;
        }
    }

    return result;
}

// Called each loop iteration
bool Application::Update() {

    bool ret = true;
    PrepareUpdate();

    if (input->GetWindowEvent(WE_QUIT) == true)
        ret = false;

    if (ret == true)
        ret = PreUpdate();

    if (ret == true)
        ret = DoUpdate();

    if (ret == true)
        ret = PostUpdate();

    FinishUpdate();
    return ret;
}

// Called before quitting
bool Application::CleanUp() {
    LOG("Application::CleanUp");

    //Iterates the module list and calls CleanUp on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->CleanUp();
        if (!result) {
            break;
        }
    }

    return result;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
}

// ---------------------------------------------
void Application::FinishUpdate()
{

}

// Call modules before each loop iteration
bool Application::PreUpdate()
{
    //Iterates the module list and calls PreUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PreUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules on each loop iteration
bool Application::DoUpdate()
{
    //Iterates the module list and calls Update on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Update(dt);
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules after each loop iteration
bool Application::PostUpdate()
{
    //Iterates the module list and calls PostUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PostUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}