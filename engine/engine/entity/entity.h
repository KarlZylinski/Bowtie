#pragma once

namespace bowtie
{

// An entity is just an integer. Internally (in the entity manager), it is split into two parts.
// The first 20 bits is the entity index and the last 12 it's generation.
typedef unsigned Entity;

}