cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

add_executable(HelloWorld helloworld.cpp Application.cpp Application.h Module.h Window.cpp Window.h Render.cpp Render.h Input.cpp Input.h )


target_link_libraries(MyEngine PRIVATE SDL3::SDL3 SDL3_image::SDL3_image fmt::fmt)