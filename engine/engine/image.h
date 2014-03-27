#pragma once

#include "resource_handle.h"
#include <foundation/vector2u.h>

namespace bowtie
{

struct Image
{
	ResourceHandle resoure_handle;
	Vector2u size;
};

}