
## Initial work
 - [x] Figure out how to call C++ from JS & visa versa

## Injesting a BSP
(http://www.mralligator.com/q3/, http://graphics.cs.brown.edu/games/quake/quake3.html)
 - [x] Load a .bsp into JS memory
 - [x] Load that .bsp into C++

## Getting a WebGL pipeline started:
 - [x] Load a GLSL file
 - [x] Get GLSL version 300 to load
 - [x] Try out GLFW
 - [x] Render a triangle
 - [x] Drawing polygons (open.gl)
 - [x] Textures (open.gl)

## Ramping up WebGL:
 - [x] Draw a cube w/ WebGL
 - [x] Capture mouse input
 - [x] Capture WASD
 - [x] Fly around the cube

## Generate C++ <=> TS bindings from a schema file
 - [x] Reorganize directory structure
 - [x] Setup Typescript
 - [x] Setup schema.json
 - [x] Setup generate_bindings.py
 - [x] Generate a ".h" file and a ".ts" definitions file

## Rendering faces
 - [x] Split up setup/rendering code into different scenarios (eg test scenario, bsp scenario)
 - [x] Figure out how to load faces + vertices + edges into OpenGL
 - [x] Render faces with random colors

## Rendering textures
 - [x] During resource loading, find out what textures to download
 - [x] Load textures in the ResourceManager
 - [x] Bind textures to the GPU
 - [x] Add texture coordinates to GLSL
 - [x] Render textures
 - [ ] Look into surface + content flags (???)

## Rendering lightmaps
 - [x] Load lightmaps in the ResourceManger
 - [x] Bind lightmap texture to the GPU
 - [x] Add lightmap coordinates to GLSL
 - [x] Render lightmaps

## Rendering patches
 - [x] Implement operator* & + for vertex_t
 - [x] For each face, loop over the contained patches
 - [x] For each patch, get the control points
 - [x] Tesselate the patch based on the control points
 - [x] Bind the tesselated vertices to the GPU
 - [x] Bind the tesselated lightmap/texture coordinates
 - [x] They should render!

## Rendering script textures (shaders)
 - [x] Load the script definitions into textures_manifest.json
 - [x] Load the script definition into TS
 - [x] Add basic texture commands to schema
 - [x] Add an alpha setting to the texture commands!
 - [x] Pass texture commands from TS => C++ per texture
 - [x] Read texture commands when rendering faces, starting with alpha.
 - [x] Render transparency
   - [x] Extend render_scene shader to also allow taking in a depth-buffer
   - [x] Allow render_scene shader to blend
   - [x] Render translucent elements onto a separate buffer
   - [x] Composite the translucency buffer with the solid buffer
 - [ ] Learn more about the Quake shader pipeline? http://toolz.nexuizninjaz.com/shader/shader/section2.htm#2.1
 - [ ] Organize faces into which faces need to be drawn in which pass
 - [ ] textures/proto2/beam_blue -- uses alpha & add blend function
 - [ ] data/textures/sfx/beam.jpg -- uses scroll mod
 - [ ] data/textures/skies/meth_clouds_blue.jpg -- uses scale & scroll mod

## Improve ResourceManager
 - [x] Add a interface HasResources interface
 - [x] Refactor the scenarios etc to use it

## Hitscan
 - [x] Detect which face is closest to the player @ the crosshair location
   - [x] What's the interesection distance?
   - [x] Run on all faces (brute force)

## Add OSX as a compliation target
 - [x] Create an XCode project
 - [x] Get GLFW working in XCode
 - [x] Get a WebView showing up
 - [x] Show a triangle on the screen
 - [x] Get mouse-input working
 - [x] Allow message passing from C++ => TS in the XCode build
 - [x] Allow message passing from TS => C++ in the XCode build
 - [x] Get build working in emscripten again
 - [ ] Setup CMakelists.txt

## Random C++ stuff
 - [ ] Extract simple shared structs into a header?

## Intorduce a debugging front-end
 - [ ] Add a drop-down that lets you choose which scenario to run
   - [ ] Each resource needs to be cleaned up.
 - [ ] Maybe setup React? Mirror some state from C++ into a React store.
 - [ ] Or... create a console?

## Physics
 - [ ] Integrate ReactPhysics3D
 - [ ] Generate static collision objects based on BSP mesh
 - [ ] Generate dynamic collision object for the player
 - [ ] WASD should accelerate the player
 - [ ] :O

## Player model + 3rd person view

## Animating player model

## HUD

## VQ3 physics (strafe jumping!)

## Bots that can navigate

## Railgun

## Lightning gun

## Rockets

## Bots that can fight back!

## Partical effects (explosions)

## Sounds (weapons, footsteps)

## Spatial sound engine

## Game scripting (lua?)
