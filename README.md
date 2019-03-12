# Vopix Engine

![Logo](../assets/logo.png?raw=true)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/d588f613f206400b847e09bcc129b298)](https://www.codacy.com?utm_source=github.com&utm_medium=referral&utm_content=KellerMartins/PixelVoxels&utm_campaign=Badge_Grade)
[![CodeFactor](https://www.codefactor.io/repository/github/kellermartins/pixelvoxels/badge)](https://www.codefactor.io/repository/github/kellermartins/pixelvoxels)

A experimental 3D engine that renders voxels as pixel art.

<a href="https://kellermartins.github.io/assets/videos/Vopix_video_720p.mp4"><img src="../assets/preview.gif" width="341" height="192" /></a>

This project was created during my study on computer graphics, the C programming language, and the inner works of an game engine over the last year. With its purpose fulfilled, I decided to provide the source code as reference to others who might want to learn from it, as even if it is not exactly architectured like a full blown engine, many of the systems implemented here are present in a way or another in those engines.

## Features

-   ECS based architecture
-   Scene and Prefab editor
-   Simple kinematic and dynamic physics
-   Lua scripting
-   .vox File loading
    -   Single object
    -   Multiple objects with position and rotation
-   Multiple voxel objects rendering in an isometric pixel art style
    -   OpenGL renderer
    -   Outline, vignette and chromatic aberration effects
    -   Point lights and directional light

## Building

### Dependencies
-   SDL, SDL image, SDL ttf
-   GLEW
-   Lua5.3

### Commands
-   `make`
    -   Build the engine
    -   On Windows: modify INCLUDE_PATHS and LIBRARY_PATHS to the location of the libraries in your system
-   `clean`
    -   Remove the `./Build` folder
-   `tcc`
    -   Compile and run the engine with the Tiny C Compiler
    -   Has some runtime issues when moving objects with the move gizmos
