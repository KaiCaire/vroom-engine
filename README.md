# Vroom Engine

Geometry visualizer and game engine editor.
[Github link](https://github.com/KaiCaire/vroom-engine)

## Description

Including an orbital camera and a basic editor for easier use, this version consists of an engine editor, capable of importing different models (via drag and drop or assets viewer) and modifying scene objects.

## Getting Started

### Camera Controls

* Free Look (WASDQE) is enabled by holding the Right Mouse Button.
* Move Forward/Backward: W / S.
* Move Left/Right: A / D.
* Move Up/Down: E / Q.
* Accelerate Movement: Hold LShift.
* Orbital Rotation: ALT + Left Mouse Button.
* Panning: ALT + Middle Mouse Button (Scroll Wheel Click).
* Zoom in Depth: ALT + Right Mouse Button.
* Scroll Zoom: Use the Mouse Wheel.
* Focus on Selected Object: Press F.

### GUI and Settings

* File
   * -> Save Scene: Save the current scene
   * -> Load Scene: Load the most recent save
   * -> Exit: Exit the program
   * -> Play: Displays the camera view (see below for more information on Camera Game Objects)
      * If Play is activated, button changes to display Stop in order to switch back to the editor camera view
* View: Toggle on/off viewing different GUI windows
    * Assets Viewer: showcases all the available assets in the Assets folder
       * By clicking the expandable arrows, the contents of different folders can be displayed or hidden
       * By clicking and dragging different files, files can be placed withing different sub-folders
       * If the Asset Viewer is selected while a drag and drop of an external file is done, file is only added to Assets, not to the scene, for more control
       * By dragging a model (for example a .fbx) into the Scene window, the model is added to the scene
       * Having a game object selected (with a texture component), a texture (for example a .png) can be drag and dropped to the Texture header of the Inspector to apply that texture to the selected object
       * -> Search: a simple text search to find files with that name (note: sub-folders must be open to see the results)
    * Console: showcases all LOG messages and possible errors
      * -> Clear: clears all available LOG messages
      * -> Go to Bottom: automatically scrolls to the bottom of the console to see the latest message
    * Configuration: showcases current FPS, hardware information, memory consuption information and versions for utilized software
      * -> Fullscreen: toggle fullscreen mode on/off
      * -> Resolution: pick a window resolution
      * -> Draw Z-Buffer: activate Z-Buffer visualization
      * -> Show Object AABBs: display the bounding boxes of different objects in the scene
    * Game: Window displaying the view of the most recently added camera object in the scene
    * Hierarchy: showcases current scene's game objects
      * By clicking on the expandable arrows, children of a game object are shown/hidden
      * Clicking on a particular game object makes its information available on the Inspector window (see below)
      * Right click on a selected game object displays the Delete option in order to delete a game object
      * Clicking and dragging different game objects allows for parenting/reparenting 
      * -> Create...: create a new game object from available options
        * -> Empty: create an empty game object
        * -> Camera: create a camera game object
           * The most recently created camera object's view will be displayed in the Game window and when Play is activated
        * -> Cube: create a cube game object
    * Inspector: showcases a game object's information once it has been selected from the Hierarchy window
      * -> Game Object Name: by typing in this space, the game object's name will be changed
      * Transform: displays the game object's position, rotation and scale values
         * Each of these spaces are able to be modified by simply typing in the corresponding text box
      * Mesh: displays the game object's vertices and indices counts
        * -> Show Vertex Normals: displays (or turns off display of) the game object's vertex normals
        * -> Show Face Normals: displays (or turns off display of) the game object's face normals
      * Texture: shows game object's texture's path, width and height
    * Scene: Main editor camera view
       * Camera controls work while this window is selected
       * Clicking on a game object will select it
       * When an object is selected, its Transform gizmo is displayed
          * Position gizmo is originally displayed
          * To switch between them:
             * W -> displays position gizmo
             * E -> displays rotation gizmo
             * R -> displays scale gizmo
       * Drag and drop scene actions
          * 3D models Drag & Drop (.fbx/.obj) from any File Explorer directory
          * Texture Drag & Drop (.png/.jpg/.tga/.dds) onto selected GameObjects from the Inspector
* Help
  * -> Documentation: opens this ReadMe
  * -> Report a Bug: opens the project's [Issues](https://github.com/KaiCaire/vroom-engine/issues) page
  * -> Latest Release: opens the project's [Releases](https://github.com/KaiCaire/vroom-engine/releases) page
  * -> About: opens window with information about the program

### Adiitional Actions

* Ctrl + S: Save the current scene
* Ctrl + L: Load the most recent save


## Additional Features

* Rename selected GameObject
* Delete GameObjects
* Textures with transparency
* Shaders read from external files
* Z-Buffer visualization

  
## Authors

* Ivan Alvarez -> [ivalpe](https://github.com/Ivalpe)
* Kai Caire -> [KaiCaire](https://github.com/KaiCaire)
* Lara Guevara -> [LaraGuevara](https://github.com/LaraGuevara)
* Bernat Loza -> [BerniF0x](https://github.com/BerniF0x)
* Marti Mach -> [0psycada](https://github.com/0psycada)

## Created Using The Following Libraries
* vcpkg
* sdl-3 & sdl-3.image
* glad (for OpenGL)
* assimp
* glm
* stb
* imgui (and ImGuizmo)

