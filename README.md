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
* [ ] Implement Lua support
** [ ] Create a C++-side world which has a handle to the render world
** [ ] Create a world script interface which returns a world ptr
** [ ] Create a view script interface which returns a view ptr
** [/] Create sprite script interface
** [ ] Make sprite SI take a world an manipulate that
** [ ] Create a rendering interface which takes a world, view and all that jazz and then sends rendering commands using the rendering handles of those