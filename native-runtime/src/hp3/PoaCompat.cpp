#include "Precomp.h"
#include "PoaCompat.h"
#include "PoaMath.h"
#include "VM/NativeFunc.h"
#include "Packages/Core/UObject.h"
#include "Packages/Core/UStruct.h"
#include "Packages/Core/Properties/UStructProperty.h"
#include "Packages/Core/Properties/UFloatProperty.h"
#include "Package/PackageManager.h"
#include "Engine.h"
#include <cstring>
#include <algorithm>
#include <cctype>

namespace PoaCompat
{
// EU Core.InterpCurve contains one dynamic array, Points. Point fields are
// resolved from runtime reflection below rather than assuming float offsets.
struct InterpCurve { ScriptArray Points; };

static void QuatInvert(const quaternion& q, quaternion& result)
{
    result = inverse(q); // UE rotation-quaternion inverse (conjugate).
}
static void QuatFindBetween(const vec3& a, const vec3& b, quaternion& result)
{
    result = length(a) == 0.0f || length(b) == 0.0f ? quaternion() : rotation_between(a, b);
}
static void CurveRange(const InterpCurve& curve, const char* component, float& minimum, float& maximum)
{
    minimum = maximum = 0.0f;
    if (curve.Points.GetSize() == 0) return;
    auto property = UObject::TryCast<UStructProperty>(curve.Points.GetType());
    if (!property || !property->Struct) Exception::Throw("PoA interpolation curve points are not structs");
    UProperty* member = nullptr;
    for (auto field : property->Struct->Properties)
        if (field->Name == component) { member = field; break; }
    const size_t pitch = property->ElementPitch();
    if (!member || !UObject::IsType<UFloatProperty>(member) || member->ArrayDimension != 1 ||
        member->DataOffset.DataOffset > property->Struct->StructSize ||
        property->Struct->StructSize - member->DataOffset.DataOffset < sizeof(float) ||
        property->Struct->StructSize > pitch || !curve.Points.GetData())
        Exception::Throw("Invalid PoA interpolation curve point layout");
    auto data = static_cast<const uint8_t*>(curve.Points.GetData());
    for (size_t i = 0; i < curve.Points.GetSize(); ++i)
    {
        float value;
        std::memcpy(&value, data + i * pitch + member->DataOffset.DataOffset, sizeof(value));
        if (i == 0) minimum = maximum = value;
        else { minimum = std::min(minimum, value); maximum = std::max(maximum, value); }
    }
}
static void InterpCurveGetOutputRange(const InterpCurve& curve, float& minimum, float& maximum)
{
    CurveRange(curve, "OutVal", minimum, maximum);
}
static void InterpCurveGetInputDomain(const InterpCurve& curve, float& minimum, float& maximum)
{
    CurveRange(curve, "InVal", minimum, maximum);
}
static void GetIniEntry(const std::string& section, const std::string& key,
                        const std::string& package, std::string& result)
{
    std::string ini = package.empty() ? "system" : package;
    if (ini.size() >= 4)
    {
        std::string suffix = ini.substr(ini.size() - 4);
        std::transform(suffix.begin(), suffix.end(), suffix.begin(), [](unsigned char c) { return std::tolower(c); });
        if (suffix == ".ini") ini.resize(ini.size() - 4);
    }
    result = engine->packages->GetIniValue(ini, section, key, "");
}
void RegisterNatives()
{
    RegisterVMNativeFunc_2("Object", "QuatInvert", &QuatInvert, 0);
    RegisterVMNativeFunc_3("Object", "QuatFindBetween", &QuatFindBetween, 0);
    RegisterVMNativeFunc_3("Object", "InterpCurveGetOutputRange", &InterpCurveGetOutputRange, 0);
    RegisterVMNativeFunc_3("Object", "InterpCurveGetInputDomain", &InterpCurveGetInputDomain, 0);
    RegisterVMNativeFunc_4("Object", "GetIniEntry", &GetIniEntry, 0);
}
}
