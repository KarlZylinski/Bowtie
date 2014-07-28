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
* [/] Render world
* [/] State reflection of sprites
* [/] Implement Lua support
    * [x] Create a C++-side world which has a handle to the render world
    * [/] Make it possible to specify dependencies (later to be dependencies on a level). I need a texture to load a sprite, and I'm not sure when or where to get that texture.
    * [x] Create a world script interface which returns a world ptr
    * [ ] Create a view script interface which returns a view ptr
    * [/] Create sprite script interface
    * [/] Make sprite SI take a world an manipulate that
    * [/] Create a rendering interface which takes a world, view and all that jazz and then sends rendering commands using the rendering handles of those
* [ ] Sorting depending on behind/infront
* [ ] Mesh based occlusion
    * [ ] Check if player is in triangle
    * [ ] Make editor to plot mesh (level editor???)
    * [ ] Set sort value depending on player occlusion
* [ ] Materials w/ shaders
* [x] Split engine and renderer into two
* [ ] Text rendering
    * [x] Create geometry for text
    * [ ] Update geometry when text or font changes
* [x] Make sure to remove hacks from combine_rendered_worlds and split it into multiple methods. Seems to be  some haze with arrays of textures...
* [x] Change render resource handles in render_world to drawables