#include "MemoryManager.h"
#include "BSThreadEvent.h"
#include <TiltedOnlinePCH.h>
#include <mimalloc.h>
#include <Games/References.h>

std::byte* g_trash{ nullptr };

void* AllocateNew(void* a_self, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired) {

    if(a_size <= 0)
        return g_trash;

    switch (a_size)
    {
        case sizeof(Actor): a_size = sizeof(ExActor); break;
        case sizeof(PlayerCharacter): a_size = sizeof(ExPlayerCharacter); break;
        default: break;
    }

    auto* pPointer = a_alignmentRequired ?
                     mi_malloc_aligned(a_size, a_alignment) :
                     mi_malloc(a_size);

    if (!pPointer)
        return nullptr;

    ActorExtension* pExtension = nullptr;

    switch (a_size)
    {
        case sizeof(ExActor): pExtension = static_cast<ActorExtension*>(static_cast<ExActor*>(pPointer)); break;
        case sizeof(ExPlayerCharacter): pExtension = static_cast<ActorExtension*>(static_cast<ExPlayerCharacter*>(pPointer)); break;
        default: break;
    }

    if (pExtension)
    {
        new (pExtension) ActorExtension;
    }

    return pPointer;

}

void DeallocateNew(void* a_self, void* a_mem, bool a_alignmentRequired)
{
    if (a_mem != g_trash)
        mi_free(a_mem);
}

void* ReallocateNew(void* a_self, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment,
                 bool a_alignmentRequired)
                 {
    if (a_oldMem == g_trash)
        return AllocateNew(a_self, a_newSize, a_alignment, a_alignmentRequired);
    else
        return a_alignmentRequired ?
               mi_realloc_aligned(a_oldMem, a_newSize, a_alignment) :
               mi_realloc(a_oldMem, a_newSize);
}


MemoryManager* MemoryManager::GetSingleton()
{
    POINTER_SKYRIMSE(MemoryManager*, s_instance, 11141);
    return *s_instance.Get();
}

[[nodiscard]] void* MemoryManager::Allocate(std::size_t a_size, std::int32_t a_alignment, bool a_alignmentRequired)
{
    TP_THIS_FUNCTION(TAllocate, void*, MemoryManager, std::size_t, std::int32_t, bool);
    POINTER_SKYRIMSE(TAllocate, realAllocate, 68115);

    return TiltedPhoques::ThisCall(realAllocate, this, a_size, a_alignment, a_alignmentRequired);
}

void MemoryManager::Deallocate(void* a_mem, bool a_alignmentRequired)
{
    TP_THIS_FUNCTION(TDeallocate, void, MemoryManager, void*, bool);
    POINTER_SKYRIMSE(TDeallocate, realDeallocate, 68117);

    TiltedPhoques::ThisCall(realDeallocate, this, a_mem, a_alignmentRequired);
}

void MemoryManager::RegisterMemoryManager()
{
    TP_THIS_FUNCTION(TRegisterMemoryManager, void, const MemoryManager);
    POINTER_SKYRIMSE(TRegisterMemoryManager, s_RegisterMemoryManager, 36091);

    TiltedPhoques::ThisCall(s_RegisterMemoryManager.Get(), this);
}

namespace {

    namespace AutoScrapBuffer {
        void Ctor() {
            uintptr_t targetAddress = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68108)) + 0x1D;
            constexpr std::size_t size = 0x32 - 0x1D;
            TiltedPhoques::Nop(targetAddress, size);

        }

        void Dtor() {
            uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68109));

            {

                const uintptr_t dst = base + 0x12;
                constexpr std::size_t size = 0x2F - 0x12;

                TiltedPhoques::Nop(dst, size);

                // xor rax, rax -> 48 33 C0
                *reinterpret_cast<uint16_t*>(dst) = 0x3348;
                *reinterpret_cast<uint8_t*>(dst + 2) = 0xC0;

                // cmp rbx, rax -> 48 39 C3
                *reinterpret_cast<uint16_t*>(dst + 3) = 0x3948;
                *reinterpret_cast<uint8_t*>(dst + 5) = 0xC3;
            }

            {
                const auto dst = base + 0x2F;
                TiltedPhoques::Put(dst, std::uint8_t{0x74}); // jnz -> jz
            }
        }

        void Install() {
            Ctor();
            Dtor();
        }
    }

    namespace MemoryManagerHooks {

        //MEM_STRONG_INLINE void Fill(mem::pointer aEa, byte value, size_t aLength) noexcept
        //    {
        //        if (IsVAOnly(aEa)) TuneBase(aEa);
        //        mem::region(aEa, aLength).fill(value);
        //    }

        void ReplaceAllocRoutines() {

            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68115));
                TiltedPhoques::Fill(base, 0xCC, 0x248);  // 0xCC INT3
                TiltedPhoques::Jump(base, reinterpret_cast<uintptr_t>(&AllocateNew));
            }

            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68117));
                TiltedPhoques::Fill(base, 0xCC, 0x114);  // 0xCC INT3
                TiltedPhoques::Jump(base, reinterpret_cast<uintptr_t>(&DeallocateNew));
            }

            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68116));
                TiltedPhoques::Fill(base, 0xCC, 0x1F6);  // 0xCC INT3
                TiltedPhoques::Jump(base, reinterpret_cast<uintptr_t>(&ReallocateNew));
            }

        }

        void StubInit() {

            uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68121));

            TiltedPhoques::Fill(base, 0xCC, 0x1A7);  // 0xCC INT3

            TiltedPhoques::Put(base, 0xC3);  // 0xC3 RET
        }

        void Install() {
            StubInit();
            ReplaceAllocRoutines();
            MemoryManager::GetSingleton()->RegisterMemoryManager();
            BSThreadEvent::InitSDM();
        }
    }

    namespace msize
    {
        size_t Hook_msize(void* apData)
        {
            return mi_malloc_size(apData);
        }

        void Hookfree(void* apData)
        {
            mi_free(apData);
        }

        void* Hookcalloc(size_t aCount, size_t aSize)
        {
            return mi_calloc(aCount, aSize);
        }

        void* Hookmalloc(size_t aSize)
        {
            return mi_malloc(aSize);
        }

        void Hook_aligned_free(void* apData)
        {
            mi_free(apData);
        }

        void* Hook_aligned_malloc(size_t aSize, size_t aAlignment)
        {
            return mi_malloc_aligned(aSize, aAlignment);
        }

        void Install()
        {
            using T_msize = decltype(&Hook_msize);
            using Tfree = decltype(&Hookfree);
            using Tcalloc = decltype(&Hookcalloc);
            using Tmalloc = decltype(&Hookmalloc);
            using T_aligned_malloc = decltype(&Hook_aligned_malloc);
            using T_aligned_free = decltype(&Hook_aligned_free);
            T_msize Real_msize = nullptr;
            Tfree Realfree = nullptr;
            Tcalloc Realcalloc = nullptr;
            Tmalloc Realmalloc = nullptr;
            T_aligned_malloc Real_aligned_malloc = nullptr;
            T_aligned_free Real_aligned_free = nullptr;

            const char* cModuleName = "api-ms-win-crt-heap-l1-1-0.dll";

            TP_HOOK_IAT(_msize, cModuleName);
            TP_HOOK_IAT(free, cModuleName);
            TP_HOOK_IAT(calloc, cModuleName);
            TP_HOOK_IAT(malloc, cModuleName);
            TP_HOOK_IAT(_aligned_malloc, cModuleName);
            TP_HOOK_IAT(_aligned_free, cModuleName);
        }
    }

    namespace ScrapHeapHooks
    {
        void* Allocate(void* a_this, std::size_t a_size, std::size_t a_alignment)
        {
            return a_size > 0 ?
                   mi_malloc_aligned(a_size, a_alignment) :
                   g_trash;
        }

        void* Ctor(void* a_this)
        {
            std::memset(a_this, 0, 0x90);
            
            reinterpret_cast<std::uintptr_t*>(a_this)[0] = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(236607));

            return a_this;
        }

        void Deallocate(void* a_this, void* a_mem)
        {
            if (a_mem != g_trash)
                mi_free(a_mem);
        }

        void WriteHooks()
        {
            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68144));
                TiltedPhoques::Fill(base, 0xCC, 0x5E7);  // 0xCC INT3
                TiltedPhoques::Jump(base, reinterpret_cast<uintptr_t>(&Allocate));
            }

            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68146));
                TiltedPhoques::Fill(base, 0xCC, 0x13E);  // 0xCC INT3
                TiltedPhoques::Jump(base, reinterpret_cast<uintptr_t>(&Deallocate));
            }

            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68142));
                TiltedPhoques::Fill(base, 0xCC, 0x13A);  // 0xCC INT3
                TiltedPhoques::Jump(base, reinterpret_cast<uintptr_t>(&Ctor));
            }

        }

        void WriteStubs()
        {
            using tuple_t = std::tuple<unsigned long long, std::size_t>;
            const std::array todo{
                    tuple_t{ 68152, 0xBA },           // Clean
                    tuple_t{ 68151, 0x8 },   // ClearKeepPages
                    tuple_t{ 68155, 0xF6 }, // InsertFreeBlock
                    tuple_t{ 68156, 0x185 },  // RemoveFreeBlock
                    tuple_t{ 68150, 0x4 },    // SetKeepPages
                    tuple_t{ 68143, 0x32 },   // dtor
            };

            for (const auto& [offset, size] : todo)
            {
                uintptr_t base = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(offset));
                TiltedPhoques::Fill(base, 0xCC, size);  // 0xCC INT3
                TiltedPhoques::Put(base, 0xC3);  // 0xC3 RET
            }
        }

        void Install()
        {
            WriteStubs();
            WriteHooks();
        }
    }



}

void InstallMemoryPatches()
{
    g_trash = new std::byte[1u << 10]{ static_cast<std::byte>(0) };
    spdlog::info("Installing Memory patchers ...");
    AutoScrapBuffer::Install();
    MemoryManagerHooks::Install();
    msize::Install();
    ScrapHeapHooks::Install();
    spdlog::info("Memory patchers installed.");
}