#include "Precomp.h"
#include "PoaCore.h"
#include "PoaMath.h"
#include "VM/NativeFunc.h"
#include "Packages/Core/UObject.h"
#include "Native/NObject.h"

namespace PoaCore
{
static void Acos(float value, float& result) { result = std::acos(value); }
static void Asin(float value, float& result) { result = std::asin(value); }
static void QuatProduct(const quaternion& a, const quaternion& b, quaternion& result)
{
    result = Product(a, b);
}
static void QuatRotateVector(const quaternion& q, const vec3& v, vec3& result)
{
    result = Rotate(q, v);
}
static void QuatFromAxisAndAngle(const vec3& axis, float angle, quaternion& result)
{
    // A zero axis otherwise divides by zero in the shared U227 constructor.
    // Preserve its normalized-axis/radian convention for nonzero inputs.
    result = length(axis) == 0.0f ? quaternion() : quaternion(angle, axis);
}
static void GetLastStateName(UObject* self, NameString& result)
{
    result = self->GetName("LastStateName");
}

void RegisterNatives()
{
    // Argument/return shapes and native indices verified in the supplied EU Core.u.
    RegisterVMNativeFunc_2("Object", "Acos", &Acos, 0);
    RegisterVMNativeFunc_2("Object", "Asin", &Asin, 0);
    RegisterVMNativeFunc_3("Object", "Greater_StrStr", &NObject::Greater_StrStr, 116);
    RegisterVMNativeFunc_1("Object", "GetLastStateName", &GetLastStateName, 285);
    RegisterVMNativeFunc_3("Object", "QuatProduct", &QuatProduct, 0);
    RegisterVMNativeFunc_3("Object", "QuatRotateVector", &QuatRotateVector, 0);
    RegisterVMNativeFunc_3("Object", "QuatFromAxisAndAngle", &QuatFromAxisAndAngle, 0);
}
}
