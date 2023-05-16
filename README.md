# Night Sky Grass Plains OpenGL & GLSL Scene Iteration #2
## COMP3015 Games Graphics Pipelines Coursework 2
### Development Hardware
 - IDE: Visual Studio 2022
 - OS: Windows 11
### Description
#### Overview
This project is a continuation of the Night Sky and Grass Plains OpenGL and GLSL scene program from the first COMP3015 coursework. The Night Sky Grass Plains project contains a scene mostly as described by the project's title: natural features combined with an eerie atmosphere. While being viewed, the camera will rotate around the scene in order to give the user a better view of the entire scene. The scene also contains fog, which will dynamically change in depth.
#### Novelty
The uniqueness of this scene is similar in nature to that of the original Night Sky and Grass Plains scene, in that it focuses on using an ethereal and eerie atmosphere. However, unlike in the first instance where the impression of glistening moonlight is used, a somewhat more disturbing green light is used in context of the green fire implemented in the scene. This is done to increase the disturbing atmosphere of the scene, giving it a nuanced difference to that of the original.
#### Coursework 2 Changes
In this implementation of the Night Sky and Grass Plains scene, the three Blinn-Phong lights present in the first iteration have been replaced with three PBR based lights for this iteration. In addition, fire particles with feedback were added, forming a green flame surrounding the tree in the scene. While the skybox was successfully implemented in the first iteration with no issues, the skybox in this iteration has a minor visual artifact, likely caused by the PBR lighting model.
### Architecture
#### Overview
This program contains 3rd party code written specifically for the related module by the organisation running it. That is, the project template base project. Expectable libraries are also present in this program. None of the aforementioned code is directly discussed; only that which has been written by the author of this program is discussed.
#### Scenebasic Uniform Header & CPP Files
The Scenebasic Uniform .h and .cpp files contain all code that is run on the CPU. Scenebasic Uniform itself loads models into the scene, compiles the various shader programs used and sets the uniform variables that said shader programs are dependent upon.
#### Shaders
##### Main Program
There are three shader programs that run in this implementation. The basic_uniform.vert and basic_uniform.frag shaders run on the main program named 'prog.' This shader program maintains the majority of features in the project: the maple leaf projected texture, vertex lighting, Phong based fog and PBR lighting.
##### Skybox Program
In the first iteration, the skybox was generated in the main shader program, however due to perceived incompatibility with the PBR lighting model, the creation of the skybox in the scene was moved to its own shader program named the 'skyBox' program. In order to maintain the effect of fog on the skybox, a mirrored implementation of fog from the main shader program was also implemented into the skybox shader program.
##### Fire Program
The fire shader program maintains the sole purpose of rendering the fire particles in the scene. It updates the particles' transparency based on their age and determines their rendered positions and velocities.
### Demonstrations
 - [YouTube Video](https://www.youtube.com/watch?v=Mw0wJh4uw8w)
 - [GitHub Repository](https://github.com/AmethystBird/COMP3015CW2)
### Files
 - VisualStudioProject zip file contains the Visual Studio Project
 - ExecutableProgram zip file contains an executable project with associated files
### References
 - [Skybox](https://www.pngegg.com/en/png-exlhy)
 - [Maple Leaves](https://www.rawpixel.com/image/2613522/free-illustration-image-leaves-flower-autumn)
