#pragma once

namespace bowtie
{

struct Timer
{
    void (*start)();
    real32 (*counter)();
};

}
