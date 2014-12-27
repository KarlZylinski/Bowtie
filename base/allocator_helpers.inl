inline void *data_pointer(Header *header, uint32 align) {
    auto p = header + 1;
    return memory::align_forward(p, align);
}

inline Header *header(void *data)
{
    auto p = (uint32 *)data;
    while (p[-1] == HEADER_PAD_VALUE)
        --p;
    return (Header *)memory::pointer_sub(p, sizeof(Header));
}

inline void fill(Header *header, void *data, uint64 size)
{
    header->size = size;
    #if defined(TRACING)
        header->tracing_marker = TRACING_MARKER;
    #endif

    auto p = (uint64 *)memory::pointer_add(header, sizeof(Header));
    while (p < data)
        *p++ = HEADER_PAD_VALUE;
}

inline uint64 size_with_padding(uint64 size, uint32 align) {
    return size + align * 2 + sizeof(Header);
}
