#include "memory_pool.h"

#include <sys/mman.h>


namespace tn {
namespace stitching {
MemoryPool MemoryPool::instance_;

MemoryPool::MemoryPool()
{

}
MemoryPool::~MemoryPool()
{
    releaseAllMemToOs();
}

void* MemoryPool::allocate(size_t bytes, bool take)
{
    for (auto iter = memory_blocks_.begin(); iter != memory_blocks_.end(); ++iter)
    {
        if (iter->size >= bytes && iter->idle) {
            iter->idle = false;
            return iter->address;
        }
    }
    auto addr = mmap64(nullptr, bytes, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    memory_blocks_.emplace_back(MemoryBlock(addr, bytes, !take)) ;
    return addr;
}

void MemoryPool::deallocate(void* address)
{
    for (auto iter = memory_blocks_.begin(); iter != memory_blocks_.end(); ++iter)
    {
        if (iter->address == address) {
            iter->idle = true;
            break;
        }
    }
}

void MemoryPool::releaseAllMemToOs()
{
    for (auto& block : memory_blocks_)
        munmap(block.address, block.size);
    memory_blocks_.clear();
}

} // namespace tn {
} // namespace stitching {