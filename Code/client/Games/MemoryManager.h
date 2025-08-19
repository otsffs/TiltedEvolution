
class MemoryManager {
public:
    static MemoryManager* GetSingleton();
    void RegisterMemoryManager();

    void *Allocate(unsigned __int64 a_size, int a_alignment, bool a_alignmentRequired);

    void Deallocate(void *a_mem, bool a_alignmentRequired);
};

void InstallMemoryPatches();

void* AllocateNew(void* a_self, std::size_t a_size, std::uint32_t a_alignment = 0, bool a_alignmentRequired = false);

void DeallocateNew(void* a_self, void* a_mem, bool a_alignmentRequired);
