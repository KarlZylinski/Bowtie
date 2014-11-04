Bowtie
======

My 2D engine, built from the ground up using C++ and OpenGL.


## The big C-style refactor

- [x] Remove render target ctor and free texture in renderer.cpp
    - [x] During rw cleanup, look at rw dstr and the texture stuff there.

- [ ] Create an entity component system
    - [ ] Rectangle rendering component
        - [x] Updating / creating needs support for updating / creating many resources at once
        - [x] Make on-demand creation of geometry support multiple rects
        - [x] Material
    - [ ] Transform component
        - [ ] Link up with rectangle renderer. How to do update by copying in transform positions when indicies aren't matching?
    - [ ] Sprite rendering component

- [ ] Turn engine into namespace of functions which operates on engine struct.
- [ ] Turn font into namespace of functions which operates on font struct.
- [ ] Turn render_interface into namespace of functions which operates on render_interface struct.
- [ ] Rename resource_manager to resource_store and have functions which operate on it in namespace
    - [ ] Optionally remove load functions and add auto-load enum
- [ ] Convert view
- [ ] Convert game
- [ ] Convert window
- [x] Convert render_world
- [ ] Convert callstack capturer to struct of function ptrs
- [x] Convert render_material
    - [ ] Have uniforms on main thread as well and keep them in a flat index-based list in renderer? Otherwise we cannot know what type uniforms are before sending update messages and we cannot fetch the values from lua.
- [x] Convert resource_lut
- [ ] Convert renderer to struct + namespace and ditch irenderer for struct of func ptrs.
- [ ] Rewrite allocators, throw out construct / destroy
    - [ ] Add page allocator?
- [ ] Array -> Vector, also shape up Vector to match new allocator shizmajings
- [ ] Make reflection message data for 1 array into soa
- [ ] Write a data compiler by moving all the loading code from resource_manager to a compiler file. Have reasource manager open compiled resources and
        throw the data into memory.
- [ ] Replace load functions in resource manager with only getter (maybe add auto load feature?). Introduce packages which specify which resources should be loaded. Have a boot package and then have levels/other types of packages do the rest. Or maybe specify default package from main.lua?
- [ ] Move all struct defs into _type.h files?
- [ ] Rename foundation to base
- [ ] Redo console as external application


## Style

- Only do dumb data assignment for structs in constructors
- Prefer namespaces of functions which operate on structs
- Prefer splitting things into separate arrays of similar data to monolithic things
- Prefer enums to bools
- Templates are only used for collection classes.


## Todo

- [ ] Deffered / heightmap lighting
- [ ] Replace PNG with homebrew DDS loader
- [ ] Instead of sending geometry for drawables, generate per frame. Do some batching if possible.
- [ ] Proper assert() and error()
- [ ] Data compiler? : D


## Old todo

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

