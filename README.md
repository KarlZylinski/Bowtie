Bowtie
======

My 2D engine, built from the ground up using C++ and OpenGL.


## The big C-style refactor

[x] Remove render target ctor and free texture in renderer.cpp
    [] During rw cleanup, look at rw dstr and the texture stuff there.

[] Split drawable into separate data arrays where drawable is index into those. Use a drawable pool to avoid fragmentation.
    [] Figure out how drawable and its "behaviors" can be split like so.
[] Convert render_world. In here store the different drawable data and drawables as indices into these. Keep an array of free entries.

[] Turn engine into namespace of functions which operates on engine struct.
[] Turn font into namespace of functions which operates on font struct.
[] Turn render_interface into namespace of functions which operates on render_interface struct.
[] Rename resource_manager to resource_store and have functions which operate on it in namespace
    [] Optionally remove load functions and add auto-load enum
[] Convert view
[] Array -> Vector
[] Rewrite allocators?
[] Convert game
[] Convert window
[] Convert callstack capturer to struct of function ptrs
[x] Convert render_material
    [] Have uniforms on main thread as well and keep them in a flat index-based list in renderer? Otherwise we cannot know what type uniforms are before sending update messages and we cannot fetch the values from lua.
[] Convert resource_lut
[] MAYBE convert renderer to struct + namespace and ditch irenderer for struct of func ptrs.

## Style

- Only do dumb data assignment for structs in constructors
- Prefer namespaces of functions which operate on structs
- Prefer splitting things into separate arrays of similar data to monolithic things
- Prefer enums to bools
- Skip references, use only pointers
- Templates are only used for collection classes.

## Todo

* [x] Renderer on separate thread
* [x] Resource manager for loading stuff
* [x] Shader loading
* [x] Image loading
* [x] Sprite rendering
* [x] Correctly scale sprites
* [x] Render world
* [x] State reflection of sprites
* [x] Implement Lua support
    * [x] Create a C++-side world which has a handle to the render world
    * [/] Make it possible to specify dependencies (later to be dependencies on a level). I need a texture to load a sprite, and I'm not sure when or where to get that texture.
    * [x] Create a world script interface which returns a world ptr
    * [/] Create a view script interface which returns a view ptr
    * [x] Create sprite script interface
    * [x] Make sprite SI take a world an manipulate that
    * [x] Create a rendering interface which takes a world, view and all that jazz and then sends rendering commands using the rendering handles of those
* [x] Materials w/ shaders
* [x] Split engine and renderer into two
* [ ] Text rendering
    * [x] Create geometry for text
    * [ ] Update geometry when text or font changes
* [x] Make sure to remove hacks from combine_rendered_worlds and split it into multiple methods. Seems to be  some haze with arrays of textures...
* [x] Change render resource handles in render_world to drawables
* [x] Make ResourceHandle::handle -> RenderResourceHandle (unsigned typedef ?). RenderResourceHandle -> RenderResource (remove render_ prefix). Move void* in ResourceHandle to Resource struct.
* [x] Shader refreshing
* [x] Remove bad lambdas
* [x] Depth sorting
* [x] Parenting with local transform and all that jazz
* [x] Break engine and gameplay script and resources apart. Make exe load a settings file which specifies lua boot file, or just always load main.lua or something. Also provide a command line option for specifying where the exe is (so you can debug from vs when devving).
* [x] Move game code to separate repository.
* [ ] Deffered lighting
