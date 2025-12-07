cmake_minimum_required(VERSION 3.10)

project(VroomEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_HAS_STD_BYTE=0)


find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)



add_executable(VroomEngine src/VroomEngine.cpp src/Application.cpp src/Application.h src/Module.h src/Window.cpp src/Window.h src/Render.cpp src/Render.h src/Input.cpp src/Input.h src/OpenGL.h src/OpenGL.cpp src/Log.h src/Log.cpp src/FileSystem.h src/FileSystem.cpp src/Shader.h src/Shader.cpp src/ResourceMesh.h src/ResourceMesh.cpp src/ModelImporter.h src/ModelImporter.cpp src/ResourceTexture.h src/ResourceTexture.cpp src/Component.h src/Component.cpp src/GameObject.h src/GameObject.cpp src/TransformComponent.h src/TransformComponent.cpp src/RenderMeshComponent.h src/RenderMeshComponent.cpp src/MaterialComponent.h src/MaterialComponent.cpp src/Camera.h src/Camera.cpp src/GUIManager.h src/GUIManager.cpp src/GUIElement.cpp src/GUIElement.h src/UUID.h src/UUID.cpp src/Resource.h src/Resource.cpp src/MeshImporter.h src/MeshImporter.cpp src/Importer.h src/Importer.cpp src/TextureImporter.h src/TextureImporter.cpp src/Scene.h src/Scene.cpp src/SceneManager.h src/SceneManager.cpp src/ResourceManager.h src/ResourceManager.cpp src/Octree.cpp src/Octree.h)




target_link_libraries(VroomEngine PRIVATE SDL3::SDL3 SDL3_image::SDL3_image fmt::fmt glad::glad assimp::assimp glm::glm imgui::imgui nlohmann_json::nlohmann_json)