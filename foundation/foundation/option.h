#pragma once

#include <functional>

namespace bowtie
{

template<typename T>
struct Option
{
	bool is_some;
	T value;
};

namespace option
{

template<typename T> Option<T> some(T value)
{
	Option<T> o;
	o.is_some = true;
	o.value = value;
	return o;
}

template<typename T> Option<T> none()
{
	Option<T> o;
	o.is_some = false;
	return o;
}

template<typename T> T get(Option<T> o)
{
	assert(o.is_some && "Tried to get value of empty option");
	return o.value;
}

template<typename T> T get_value_or_null(Option<T> o)
{
	return o.is_some
		? o.value
		: nullptr;
}

} // namespace option

} // namespace bowtie