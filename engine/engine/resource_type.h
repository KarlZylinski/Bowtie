#pragma once

namespace bowtie
{

namespace resource_type
{

enum ResourceType
{
	Shader, Image, Sprite, Texture, Font, Drawable, RenderMaterial, NumResourceTypes
};

}

typedef resource_type::ResourceType ResourceType;

}
