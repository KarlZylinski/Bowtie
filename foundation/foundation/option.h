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

template<typename T> T get_or_default(Option<T> o, std::function<T()>& default_creator)
{
	return o.is_some
		? o.value
		: default_creator();
}

} // namespace option

} // namespace bowtie