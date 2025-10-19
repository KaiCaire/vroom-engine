cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

add_executable(HelloWorld src/helloworld.cpp src/Application.cpp src/Application.h src/Module.h src/Window.cpp src/Window.h src/Render.cpp src/Render.h src/Input.cpp src/Input.h src/Log.cpp      # <-- AÑADE ESTA LÍNEA
     src/Log.h     )


target_link_libraries(HelloWorld PRIVATE SDL3::SDL3 SDL3_image::SDL3_image fmt::fmt)