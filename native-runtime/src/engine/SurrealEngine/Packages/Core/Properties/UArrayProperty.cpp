
#include "Precomp.h"
#include "UArrayProperty.h"
#include "UStructProperty.h"
#include "Packages/Core/UStruct.h"
#include "PropertyText.h"

UArrayProperty::UArrayProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueArray;
}

void UArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
}

void UArrayProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Inner);
}

static void ReadArray(UArrayProperty* property, void* data, ObjectStream* stream, int64_t expectedSize)
{
    const uint32_t start = stream->Tell();
    if (!property->Inner || expectedSize == 0) Exception::Throw("Invalid serialized array property");
    const int count = stream->ReadIndex();
    if (count < 0 || size_t(count) > stream->Remaining()) Exception::Throw("Invalid serialized array length");
    if (expectedSize >= 0)
    {
        const uint64_t consumed = stream->Tell() - start;
        if (consumed > uint64_t(expectedSize) || uint64_t(count) > uint64_t(expectedSize) - consumed)
            Exception::Throw("Serialized array count exceeds its property size");
    }
    ScriptArray replacement(property->Inner);
    replacement.Resize(size_t(count));
    for (int i = 0; i < count; ++i)
    {
        property->Inner->LoadStructMemberValue(replacement.GetItem(size_t(i)), stream);
        if (expectedSize >= 0 && uint64_t(stream->Tell() - start) > uint64_t(expectedSize))
            Exception::Throw("Serialized array exceeds its property size");
    }
    if (expectedSize >= 0 && uint64_t(stream->Tell() - start) != uint64_t(expectedSize))
        Exception::Throw("Serialized array property size mismatch");
    static_cast<ScriptArray*>(data)->Swap(replacement);
}
void UArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
    ThrowIfTypeMismatch(header, UPT_Array);
    if (header.size <= 0 || header.arrayIndex < 0 || header.arrayIndex >= ArrayDimension)
        Exception::Throw("Serialized array fixed index is outside its property");
    ReadArray(this, static_cast<ScriptArray*>(data) + header.arrayIndex, stream, header.size);
}

void UArrayProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
    ReadArray(this, data, stream, -1);
}

void UArrayProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Array;
}

void UArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	ScriptArray& vec = *static_cast<ScriptArray*>(data);
	stream->WriteIndex((int)vec.GetSize());
	for (size_t i = 0, count = vec.GetSize(); i < count; i++)
	{
		Inner->SaveValue(vec.GetItem(i), stream);
	}
}

size_t UArrayProperty::ElementAlignment()
{
	return alignof(ScriptArray);
}

size_t UArrayProperty::ElementSize()
{
	return sizeof(ScriptArray);
}

void UArrayProperty::ConstructElement(void* data)
{
	new ((char*)data) ScriptArray(Inner);
}

void UArrayProperty::CopyConstructElement(void* data, const void* src)
{
	ConstructElement(data);
	CopyElement(data, src);
}

void UArrayProperty::DestructElement(void* data)
{
	auto vec = static_cast<ScriptArray*>(data);
	vec->~ScriptArray();
}

void UArrayProperty::CopyElement(void* data, const void* src)
{
	auto& vec = *static_cast<ScriptArray*>(data);
	auto& srcvec = *static_cast<const ScriptArray*>(src);
	vec = srcvec;
}

bool UArrayProperty::CompareElement(const void* a, const void* b)
{
	auto& avec = *static_cast<const ScriptArray*>(a);
	auto& bvec = *static_cast<const ScriptArray*>(b);
	return avec == bvec;
}

bool UArrayProperty::CompareLessElement(const void* a, const void* b)
{
	auto& avec = *static_cast<const ScriptArray*>(a);
	auto& bvec = *static_cast<const ScriptArray*>(b);
	return avec.GetSize() < bvec.GetSize();
}

void UArrayProperty::GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i)
{
	if (i >= ArrayDimension)
		Exception::Throw("UArrayProperty::GetExportText index out of bounds");

	size_t elementPitch = ElementPitch();
	int offset = i * (int)elementPitch;

	ScriptArray* objarray = static_cast<ScriptArray*>(obj->GetProperty(Name)) + offset;
	ScriptArray* defarray = (defobj) ? static_cast<ScriptArray*>(defobj->GetProperty(Name)) + offset : nullptr;

	for (int k = 0; k < objarray->GetSize(); k++)
	{
		void* objval = objarray->GetItem(k);
		void* defval = (defarray && k < defarray->GetSize()) ? defarray->GetItem(k) : nullptr;

		Inner->GetExportText(buf, whitespace, objval, defval, i);
	}
}

static bool SupportsArrayText(UProperty* property, int depth)
{
    if (!property || depth >= 64) return false;
    if (auto array = UObject::TryCast<UArrayProperty>(property)) return SupportsArrayText(array->Inner, depth + 1);
    if (auto structure = UObject::TryCast<UStructProperty>(property))
    {
        if (!structure->Struct) return false;
        for (auto member : structure->Struct->Properties)
            if (!SupportsArrayText(member, depth + 1)) return false;
        return true;
    }
    switch (property->ValueType)
    {
    case ExpressionValueType::ValueByte: case ExpressionValueType::ValueInt:
    case ExpressionValueType::ValueBool: case ExpressionValueType::ValueFloat:
    case ExpressionValueType::ValueName: case ExpressionValueType::ValueString: return true;
    default: return false;
    }
}
bool UArrayProperty::SupportsText() const
{
    return SupportsArrayText(Inner, 0);
}
std::string UArrayProperty::PrintValue(const void* data)
{
    if (!SupportsText()) Exception::Throw("Unsupported array text element type");
    const auto& array = *static_cast<const ScriptArray*>(data);
    std::string result = "(";
    for (size_t i = 0; i < array.GetSize(); ++i)
    {
        if (i) result += ',';
        result += Inner->PrintValue(static_cast<const uint8_t*>(array.GetData()) + i * Inner->ElementPitch());
    }
    return result + ')';
}
void UArrayProperty::SetValueFromString(void* data, const std::string& valueString)
{
    if (!SupportsText()) Exception::Throw("Unsupported array text element type");
    const auto values = PropertyText::List(valueString);
    ScriptArray replacement(Inner);
    replacement.Resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) Inner->SetValueFromString(replacement.GetItem(i), values[i]);
    static_cast<ScriptArray*>(data)->Swap(replacement);
}

// Repeated INI keys describe individual array elements. A single () is our
// explicit empty-array marker; string elements containing () are quoted.
void UArrayProperty::SetConfigValues(void* data, const Array<std::string>& values)
{
    if (values.size() == 1 && PropertyText::Trim(values[0]) == "()")
    {
        SetValueFromString(data, "()");
        return;
    }
    std::string text = "(";
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i) text += ',';
        if (Inner && (Inner->ValueType == ExpressionValueType::ValueString || Inner->ValueType == ExpressionValueType::ValueName))
            text += PropertyText::Quote(PropertyText::Unquote(values[i]));
        else text += PropertyText::Trim(values[i]);
    }
    SetValueFromString(data, text + ')');
}
Array<std::string> UArrayProperty::GetConfigValues(const void* data)
{
    const auto elements = PropertyText::List(PrintValue(data));
    Array<std::string> result;
    for (const auto& element : elements) result.push_back(element);
    if (result.empty()) result.push_back("()");
    return result;
}

GCAllocation* UArrayProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	ScriptArray* array = static_cast<ScriptArray*>(data);
	size_t count = array->GetSize();
	for (size_t i = 0; i < count; i++)
	{
		marklist = Inner->MarkPropertyElement(marklist, array->GetItem(i));
	}
	return marklist;
}
