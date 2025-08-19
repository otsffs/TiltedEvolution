#pragma once

struct GSysAllocBase
{
public:

    virtual ~GSysAllocBase() = default;  // 00
    // add
    virtual bool InitHeapEngine([[maybe_unused]] const void* a_heapDesc) { return false; }  // 01
    virtual void ShutdownHeapEngine() { return; }                                           // 02
};
static_assert(sizeof(GSysAllocBase) == 0x8);