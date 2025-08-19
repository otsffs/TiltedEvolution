#include "BSThreadEvent.h"
#include <TiltedOnlinePCH.h>

void BSThreadEvent::InitSDM()
{
    using TInitSDM = void (*)();

    static uintptr_t address = reinterpret_cast<uintptr_t>(VersionDb::Get().FindAddressById(68449));
    auto func = reinterpret_cast<TInitSDM>(address);
    func();

}
