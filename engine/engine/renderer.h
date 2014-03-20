#pragma once

namespace bowtie
{

class Renderer
{
public:
	Renderer();
	virtual ~Renderer();
	
	bool active() { return _active; }

	virtual void clear() = 0;
	virtual void run_render_thread() = 0;
	
protected:
	void set_active(bool active) { _active = active; }

private:
	bool _active;
};

}