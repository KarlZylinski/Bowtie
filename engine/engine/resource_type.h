#pragma once

namespace bowtie
{

namespace resource_type
{

enum ResourceType
{
	Shader, Image, Sprite, Texture, Font, Drawable, Material, NotInitialized, NumResourceTypes
};

}

typedef resource_type::ResourceType ResourceType;

}
