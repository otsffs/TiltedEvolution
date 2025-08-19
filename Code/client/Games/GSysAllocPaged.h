#pragma once

#include "GSysAllocBase.h"

struct GHeapMemVisitor;
struct GHeapSegVisitor;

struct GSysAllocPaged : public GSysAllocBase
{
public:
    struct Info
    {
    public:
        // members
        size_t minAlign;            // 00
        size_t maxAlign;            // 08
        size_t granularity;         // 10
        size_t sysDirectThreshold;  // 18
        size_t maxHeapGranularity;  // 20
        bool        hasRealloc;          // 28
    };
    static_assert(sizeof(Info) == 0x30);

    // override (GSysAllocBase)
    bool InitHeapEngine(const void* a_heapDesc) override
    {
        TP_THIS_FUNCTION(TInitHeapEngine, bool, GSysAllocPaged, const void*);
        POINTER_SKYRIMSE(TInitHeapEngine, realInitHeapEngine, 84557);

        return TiltedPhoques::ThisCall(realInitHeapEngine, this, a_heapDesc);
    }

    void ShutdownHeapEngine() override
    {
        TP_THIS_FUNCTION(TShutdownHeapEngine, void, GSysAllocPaged);
        POINTER_SKYRIMSE(TShutdownHeapEngine, realShutdownHeapEngine, 84559);
        TiltedPhoques::ThisCall(realShutdownHeapEngine, this);
    }

    // add
    virtual void  GetInfo(Info* a_info) const = 0;                                 // 03
    virtual void* Alloc(size_t a_size, size_t a_align) = 0;              // 04
    virtual bool  Free(void* a_ptr, size_t a_size, size_t a_align) = 0;  // 05
    virtual bool  ReallocInPlace(
            [[maybe_unused]] void*       a_oldPtr,
            [[maybe_unused]] size_t a_oldSize,
            [[maybe_unused]] size_t a_newSize,
            [[maybe_unused]] size_t a_align) { return false; }  // 06
    virtual void* AllocSysDirect(
            [[maybe_unused]] size_t  a_size,
            [[maybe_unused]] size_t  a_alignment,
            [[maybe_unused]] size_t* a_actualSize,
            [[maybe_unused]] size_t* a_actualAlign) { return nullptr; }  // 07
    virtual bool FreeSysDirect(
            [[maybe_unused]] void*       a_ptr,
            [[maybe_unused]] size_t a_size,
            [[maybe_unused]] size_t a_alignment) { return false; }                                            // 08
    [[nodiscard]] virtual size_t GetBase() const { return 0; }                                            // 09
    [[nodiscard]] virtual size_t GetSize() const { return 0; }                                            // 0A
    [[nodiscard]] virtual size_t GetFootprint() const { return 0; }                                       // 0B
    [[nodiscard]] virtual size_t GetUsedSpace() const { return 0; }                                       // 0C
    virtual void VisitMem([[maybe_unused]] GHeapMemVisitor* a_visitor) const { return; }  // 0D
    virtual void VisitSegments(
            [[maybe_unused]] GHeapSegVisitor* a_visitor,
            [[maybe_unused]] size_t      a_catSeg,
            [[maybe_unused]] size_t      a_catUnused) const { return; }  // 0E
};
static_assert(sizeof(GSysAllocPaged) == 0x8);
