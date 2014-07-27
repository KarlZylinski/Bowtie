#pragma once

namespace bowtie
{

namespace resource_type
{

enum ResourceType
{
	Shader, Image, Sprite, Texture, Font, NumResourceTypes, Drawable
};

}

typedef resource_type::ResourceType ResourceType;

}
