#include <Windows.h>
#include <moonsugar/memory.h>

void* ms_reserve(const size_t count) {
    return VirtualAlloc(
        NULL,
        count,
        MEM_RESERVE,
        PAGE_READWRITE
    );
}

void ms_release(void * const ptr, size_t const count) {
    VirtualFree(ptr, (SIZE_T)count, MEM_RELEASE);
}

bool ms_commit(void * const ptr, const size_t count) {
    return VirtualAlloc(
        ptr,
        count,
        MEM_COMMIT,
        PAGE_READWRITE
    ) != NULL;
}

void ms_decommit(void * const ptr, const size_t count) {
    VirtualFree(
        ptr,
        count,
        MEM_DECOMMIT
    );
}
