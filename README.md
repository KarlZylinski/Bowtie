Bowtie
======

My 2D engine, built from the ground up using C++ and OpenGL.

Todo
======

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
* [ ] Shader refreshing
* [ ] Depth sorting
* [ ] Parenting with local transform and all that jazz
* [ ] Break engine and gameplay script and resources apart. Make exe load a settings file which specifies lua boot file, or just always load main.lua or something. Also provide a command line option for specifying where the exe is (so you can debug from vs when devving).
* [ ] Move game code to separate repository.
