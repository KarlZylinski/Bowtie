#include "texture.h"

namespace bowtie
{

Texture::Texture(Image* image) : image(image), pixel_format(image->pixel_format), resolution(image->resolution)
{
}

}