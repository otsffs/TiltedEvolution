#pragma once

#include "GSysAllocPaged.h"
#include <TiltedOnlinePCH.h>
#include "MemoryManager.h"

struct HeapDescPlaceholder;

class CustomAllocator :
        public GSysAllocPaged
{
public:
    [[nodiscard]] static CustomAllocator* GetSingleton()
    {
        static CustomAllocator singleton;
        return std::addressof(singleton);
    }

protected:
    void GetInfo(Info* a_info) const override
    {
        assert(a_info != nullptr);

        a_info->minAlign = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
        a_info->maxAlign = 0;
        a_info->granularity = 1u << 16;
        a_info->sysDirectThreshold = 1;
        a_info->maxHeapGranularity = 0;
        a_info->hasRealloc = false;
    }

    void* Alloc(std::size_t a_size, std::size_t a_align) override
    {
        return _allocator->Allocate(a_size, static_cast<std::uint32_t>(a_align), true);
    }

    bool Free(void* a_ptr, std::size_t, std::size_t) override
    {
        _allocator->Deallocate(a_ptr, true);
        return true;
    }

    void* AllocSysDirect(
            std::size_t a_size,
            std::size_t a_alignment,
            std::size_t* a_actualSize,
            std::size_t* a_actualAlign) override
    {
        assert(a_actualSize != nullptr);
        assert(a_actualAlign != nullptr);

        *a_actualSize = a_size;
        *a_actualAlign = a_alignment;
        return _allocator->Allocate(a_size, static_cast<std::uint32_t>(a_alignment), true);
    }

    bool FreeSysDirect(void* a_ptr, std::size_t, std::size_t) override
    {
        _allocator->Deallocate(a_ptr, true);
        return true;
    }

private:
    CustomAllocator() = default;
    CustomAllocator(const CustomAllocator&) = delete;
    CustomAllocator(CustomAllocator&&) = delete;
    ~CustomAllocator() = default;
    CustomAllocator& operator=(const CustomAllocator&) = delete;
    CustomAllocator& operator=(CustomAllocator&&) = delete;

    MemoryManager* _allocator{ MemoryManager::GetSingleton() };
};

namespace
{
    using TInitFunc = void(void*, const HeapDescPlaceholder&, GSysAllocBase*);

    struct Hook
    {
        static void Thunk(void* apThis, const HeapDescPlaceholder& a_rootHeapDesc, GSysAllocBase*)
        {
            POINTER_SKYRIMSE(TInitFunc, s_OriginalInit, 84562);
            s_OriginalInit(apThis, a_rootHeapDesc, CustomAllocator::GetSingleton());
        }
    };

    void Install()
    {
        uintptr_t targetAddress = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(82323)) + 0x170;

        TiltedPhoques::PutCall(targetAddress, &Hook::Thunk);
    }
}

static TiltedPhoques::Initializer s_scaleformHooks([] {
    spdlog::info("Installing Scaleform Hooks...");
    Install();
    spdlog::info("Scaleform Hooks Installed.");
});
