#pragma once

namespace bowtie
{

template<typename T, typename E>
struct Result
{
	bool is_ok;
	union
	{
		T ok;
		E err;
	}
};

namespace result
{
template<typename T, typename E> Result<T, E> ok(T value)
{
	Result<T, E> r;
	r.is_ok = true;
	r.ok = value;
	return r;
}

template<typename T, typename E> Result<T, E> err(E value)
{
	Result<T, E> r;
	r.is_ok = false;
	r.err = value;
	return r;
}

} // namespace result

} // namespace bowtie
