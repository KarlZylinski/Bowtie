#pragma once

#include "resource_handle.h"

namespace bowtie
{

struct Vector2u;
struct RendererCommand;
class Allocator;

class IRenderer
{
public:
	virtual ~IRenderer() {};
	virtual void add_renderer_command(const RendererCommand& command) = 0;
	virtual ResourceHandle create_handle() = 0;
	virtual void deallocate_processed_commands(Allocator& allocator) = 0;
	virtual bool is_active() const = 0;
	virtual bool is_setup() const = 0;
	virtual const Vector2u& resolution() const = 0;
};

}
