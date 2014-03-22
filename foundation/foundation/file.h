#pragma once

namespace bowtie
{

class Allocator;

namespace file
{

char* load(const char* filename, Allocator& allocator);

}
}