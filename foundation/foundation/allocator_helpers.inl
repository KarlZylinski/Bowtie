inline void *data_pointer(Header *header, unsigned align) {
	auto p = header + 1;
	return memory::align_forward(p, align);
}

inline Header *header(void *data)
{
	auto p = (unsigned *)data;
	while (p[-1] == HEADER_PAD_VALUE)
		--p;
	return (Header *)memory::pointer_sub(p, sizeof(Header));
}

inline void fill(Header *header, void *data, unsigned size)
{
	header->size = size;
	#if defined(TRACING)
		header->tracing_marker = TRACING_MARKER;
	#endif

	auto p = (unsigned *)memory::pointer_add(header, sizeof(Header));
	while (p < data)
		*p++ = HEADER_PAD_VALUE;
}

inline unsigned size_with_padding(unsigned size, unsigned align) {
	return size + align * 2 + sizeof(Header);
}
