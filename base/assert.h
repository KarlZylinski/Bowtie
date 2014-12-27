#pragma once
#include <cassert>

#define StaticAssert(condition, message) static_assert(condition && message)
#define Assert(condition, message) assert(condition && message)
#define Error(message) assert(!message)
