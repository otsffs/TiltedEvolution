#include <Structs/Movement.h>
#include <Serialization.hpp>

using TiltedPhoques::Serialization;

bool Movement::operator==(const Movement& acRhs) const noexcept
{
    return Position == acRhs.Position &&
        Rotation == acRhs.Rotation;
}

bool Movement::operator!=(const Movement& acRhs) const noexcept
{
    return !this->operator==(acRhs);
}

void Movement::Serialize(TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    Position.Serialize(aWriter);
    Rotation.Serialize(aWriter);
    Variables.GenerateDiff(AnimationVariables{}, aWriter);
    aWriter.WriteBits(*reinterpret_cast<const uint32_t*>(&Direction), 32);
}

void Movement::Deserialize(TiltedPhoques::Buffer::Reader& aReader) noexcept
{
    Position.Deserialize(aReader);
    Rotation.Deserialize(aReader);
    Variables = AnimationVariables{};
    Variables.ApplyDiff(aReader);

    uint64_t tmp = 0;
    aReader.ReadBits(tmp, 32);
    uint32_t tmp32 = tmp & 0xFFFFFFFF;
    Direction = *reinterpret_cast<float*>(&tmp32);
}
