# Cursed Chair Scene

mi23056 - Stefan Gajic

Interactive 3D scene depicting a dark abandoned corner environment with a chair, a lamp, and a barrel. The scene
includes an interactive event where the lamp flickers and transitions to red light when triggered.

## Controls

W/A/S/D -> Move camera
Mouse -> Look around
SPACE -> Trigger event sequence
1 -> Toggle directional light
2 -> Toggle point light
3 -> Toggle lamp sway
UP/DOWN -> Increase/decrease point light intensity
G ---> Point light switches to green  
B ---> Point light switches to blue  
Q ---> Point light resets to default color

## Features

- Dynamic lighting system with directional and point light sources
- Interactive light color and intensity control via keyboard
- Timed event sequence triggered by SPACE key
- Procedural flicker effect simulating unstable lamp behavior
- Smooth free camera navigation (WASD + mouse)
- Realistic light attenuation and specular highlights

### Fundamental:

[x] Model with lighting  
[x] Two types of lighting (directional + point) with interactive color control via keyboard  
[x] SPACE --- AFTER 2s ---> Lamp flicker ---> AFTER 2s ---> Point light transitions to red

### Group A:

[ ] Frame-buffers with post-processing
[ ] Off-screen Anti-Aliasing
[ ] Parallax Mapping
[x] Bloom with the use of HDR

### Group B:

[ ] Deferred Shading
[x] Point Shadows
[ ] SSAO

### Engine improvement:

[x] Free camera movement using keyboard and mouse
[x] Event system implementation using timers

## Models:

[floor-link] - made by chat GPT
[chair-link] - https://free3d.com/3d-model/wooden-chair-38608.html
[lamp-link] - https://sketchfab.com/3d-models/old-english-street-lamp-obj-6341be23fc704d19967944359a410d68
[lightbulb-link] - made by chat GPT
[barrel12-link] - https://www.cgtrader.com/free-3d-models/food/beverage/game-ready-wooden-barrels
[old_house-link] - https://free3d.com/3d-model/abandoned-cottage-house-825251.html

## Textures

[floor-texture-link] https://polyhaven.com/a/granite_tile
[lamp-texture-link] https://polyhaven.com/a/rusted_shutter

## Other Resources

- OpenGL tutorials (LearnOpenGL)
- Course materials
