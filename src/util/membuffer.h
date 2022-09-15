/* membuffer.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */

#pragma once

/*************************************************************************
// A MemBuffer allocates memory on the heap, and automatically
// gets destructed when leaving scope or on exceptions.
**************************************************************************/

// provides some base functionality for treating a MemBuffer as a pointer
template <class T>
class MemBufferBase {
public:
    typedef T element_type;
    typedef T *pointer;

protected:
    pointer b = nullptr;
    unsigned b_size_in_bytes = 0;

public:
    // NOTE: implicit conversion to underlying pointer
    // NOTE: for fully bound-checked pointer use SPAN_S from xspan.h
    operator pointer() const { return b; }

    template <class U,
              class /*Dummy*/ = typename std::enable_if<std::is_integral<U>::value, U>::type>
    pointer operator+(U n) const {
        size_t bytes = mem_size(sizeof(T), n); // check mem_size
        return raw_bytes(bytes) + n;           // and check bytes
    }

    // NOT allowed; use raw_bytes() instead
    template <class U,
              class /*Dummy*/ = typename std::enable_if<std::is_integral<U>::value, U>::type>
    pointer operator-(U n) const = delete;

    pointer raw_bytes(size_t bytes) const {
        if (bytes > 0) {
            if __acc_very_unlikely (b == nullptr)
                throwInternalError("MemBuffer raw_bytes unexpected NULL ptr");
            if __acc_very_unlikely (bytes > b_size_in_bytes)
                throwInternalError("MemBuffer raw_bytes invalid size");
        }
        return b;
    }
};

class MemBuffer : public MemBufferBase<unsigned char> {
public:
    MemBuffer() = default;
    explicit MemBuffer(upx_uint64_t size_in_bytes);
    ~MemBuffer();

    static unsigned getSizeForCompression(unsigned uncompressed_size, unsigned extra = 0);
    static unsigned getSizeForUncompression(unsigned uncompressed_size, unsigned extra = 0);

    void alloc(upx_uint64_t size);
    void allocForCompression(unsigned uncompressed_size, unsigned extra = 0);
    void allocForUncompression(unsigned uncompressed_size, unsigned extra = 0);

    void dealloc();
    void checkState() const;
    unsigned getSize() const { return b_size_in_bytes; }

    // explicit converstion
    void *getVoidPtr() { return (void *) b; }
    const void *getVoidPtr() const { return (const void *) b; }

    // util
    void fill(unsigned off, unsigned len, int value);
    void clear(unsigned off, unsigned len) { fill(off, len, 0); }
    void clear() { fill(0, b_size_in_bytes, 0); }

    // If the entire range [skip, skip+take) is inside the buffer,
    // then return &b[skip]; else throwCantPack(sprintf(errfmt, skip, take)).
    // This is similar to BoundedPtr, except only checks once.
    // skip == offset, take == size_in_bytes
    pointer subref(const char *errfmt, size_t skip, size_t take) {
        return (pointer) subref_impl(errfmt, skip, take);
    }

private:
    void *subref_impl(const char *errfmt, size_t skip, size_t take);

    static unsigned global_alloc_counter;

    // disable copy, assignment and move assignment
    MemBuffer(const MemBuffer &) = delete;
    MemBuffer &operator=(const MemBuffer &) = delete;
    MemBuffer &operator=(MemBuffer &&) = delete;
    // disable dynamic allocation
    ACC_CXX_DISABLE_NEW_DELETE

    // disable taking the address => force passing by reference
    // [I'm not too sure about this design decision, but we can always allow it if needed]
    MemBuffer *operator&() const = delete;
};

// raw_bytes overload
template <class T>
inline T *raw_bytes(const MemBufferBase<T> &a, size_t size_in_bytes) {
    return a.raw_bytes(size_in_bytes);
}

/* vim:set ts=4 sw=4 et: */
