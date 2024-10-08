/**
* @file
* Copyright (c) 2021 Telenav, Inc
* All rights reserved
* This program is UNPUBLISHED PROPRIETARY property of Telenav.
* Only for internal distribution.
 
* @author ZhengJifu
* @date   Otc 2021 
*/

#pragma once

#include <memory>
#include <list>
#include <mutex>


namespace tn {
namespace stitching {

class MemoryPool {
public:
    static MemoryPool* Instance() {
        return &instance_;
    }

    // take: allocated memory will be marked as not idle and can be reused only after deallocate, otherwise it's idle and can be given to other allocation requests.
    void* allocate(size_t bytes, bool take = true);
    void deallocate(void* address);
    void releaseAllMemToOs();
private:
    MemoryPool();
    ~MemoryPool();
    struct MemoryBlock {
        MemoryBlock(void* addr, size_t sz, bool idl) : address(addr), size(sz), idle(idl) {}
        bool operator<(const MemoryBlock& rhs) {
            return size < rhs.size;
        }
        bool operator==(const MemoryBlock& rhs) {
            return address == rhs.address;
        }
        void* address = nullptr;
        size_t size = 0;
        bool idle = false;
    };
    std::list<MemoryBlock> memory_blocks_;
    std::mutex block_list_mutex_;;
    static MemoryPool instance_;
};

template<typename T> class StitchingAllocator : public std::allocator<T>
{
public:
     typedef size_t     size_type;
     typedef ptrdiff_t  difference_type;
     typedef T*         pointer;
     typedef const T*   const_pointer;
     typedef T&         reference;
     typedef const T&   const_reference;
     typedef T          value_type;

     template<typename T1>
     struct rebind
     {
       typedef StitchingAllocator<T1> other; 
     };

     StitchingAllocator() {}

     template<typename T1>
     StitchingAllocator(const StitchingAllocator<T1>&) {}

    // T* allocate(size_type __n) {
    //     return MemoryPool::Instance()->allocate(__n * sizeof(T));
    //     }
    T* allocate(size_type __n, const void* = static_cast<const void*>(0)) {
        return (T*)MemoryPool::Instance()->allocate(__n * sizeof(T));
        }
    void deallocate(T* __p, size_type n) {
        MemoryPool::Instance()->deallocate(__p);
    }
};

} // namespace tn {
} // namespace stitching {