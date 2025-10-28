cmake_minimum_required(VERSION 3.10)

project(VroomEngine)

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)





add_executable(VroomEngine src/VroomEngine.cpp src/Application.cpp src/Application.h src/Module.h src/Window.cpp src/Window.h src/Render.cpp src/Render.h src/Input.cpp src/Input.h src/OpenGL.h src/OpenGL.cpp src/Log.h src/Log.cpp src/FileSystem.h src/FileSystem.cpp src/Shader.h src/Shader.cpp src/Mesh.h src/Mesh.cpp src/Model.h src/Model.cpp src/Textures.h src/Textures.cpp src/Camera.h src/Camera.cpp)


target_link_libraries(VroomEngine PRIVATE SDL3::SDL3 SDL3_image::SDL3_image fmt::fmt glad::glad assimp::assimp glm::glm)