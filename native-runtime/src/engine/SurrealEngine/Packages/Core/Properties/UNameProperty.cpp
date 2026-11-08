
#include "Precomp.h"
#include "UNameProperty.h"
#include "PropertyText.h"

UNameProperty::UNameProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueName;
}

void UNameProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Name);
	*static_cast<NameString*>(data) = stream->ReadName();
}

void UNameProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<NameString*>(data) = stream->ReadName();
}

void UNameProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Name;
}

void UNameProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteName(*static_cast<NameString*>(data));
}

void UNameProperty::SetValueFromString(void* data, const std::string& valueString)
{
	*(NameString*)data = NameString(PropertyText::Unquote(valueString));
}

std::string UNameProperty::PrintValue(const void* data)
{
	const auto& name = static_cast<const NameString*>(data)->ToString();
    // Keep ordinary Unreal identifiers unchanged; quote names that would split a list.
    if (name.empty() || name.find_first_of(",()=\"'\\ \t\r\n") != std::string::npos)
        return PropertyText::Quote(name);
    return name;
}

bool UNameProperty::IsDefaultValue(void* val)
{
	return static_cast<NameString*>(val)->IsNone();
}
