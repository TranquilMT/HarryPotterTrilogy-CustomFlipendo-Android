
#include "Precomp.h"
#include "UFont.h"
#include "Textures/UTexture.h"
#include "Utils/UTF8Reader.h"
#include "Engine.h"

void UFont::Load(ObjectStream* stream)
{
	// Note: 63 package and older inherited from UTexture. Newer versions inherit from UObject
	// We must keep the old relationship intact to load the older games.
	// If the game is newer then this is not a valid texture and any properties on UTexture do not exist.

	if (stream->GetVersion() <= 63)
	{
		UTexture::Load(stream);

		pages.resize(1);
		FontPage& page = pages.front();
		page.Texture = this;
		page.Characters.resize(stream->ReadIndex());
		for (FontCharacter& character : page.Characters)
		{
			character.StartU = stream->ReadInt32();
			character.StartV = stream->ReadInt32();
			character.USize = stream->ReadInt32();
			character.VSize = stream->ReadInt32();
		}
	}
	else
	{
		UObject::Load(stream);

		poaLayout = engine->LaunchInfo.IsHarryPotter3() && stream->GetVersion() >= 122;
		if (poaLayout)
		{
			// EU packages 122/129: global glyph array (four ints plus atlas byte).
			// Bundled 99/120/121 font packages retain the legacy page-first layout.
			// then the texture array. UE1 instead stores glyph arrays inside each page.
			int count = stream->ReadIndex();
			if (count < 0 || count > 65536)
				Exception::Throw("Invalid PoA font glyph count");
			poaCharacters.resize(count);
			poaTextureIndices.resize(count);
			for (int i = 0; i < count; ++i)
			{
				auto& c = poaCharacters[i];
				c.StartU = stream->ReadInt32();
				c.StartV = stream->ReadInt32();
				c.USize = stream->ReadInt32();
				c.VSize = stream->ReadInt32();
				poaTextureIndices[i] = stream->ReadUInt8();
			}
			count = stream->ReadIndex();
			if (count < 0 || count > 256)
				Exception::Throw("Invalid PoA font atlas count");
			pages.resize(count);
			for (auto& page : pages)
				page.Texture = stream->ReadObject<UTexture>();
			for (size_t i = 0; i < poaCharacters.size(); ++i)
				if (poaTextureIndices[i] >= pages.size() &&
					(poaCharacters[i].USize != 0 || poaCharacters[i].VSize != 0))
					Exception::Throw("Invalid PoA font atlas index");
		}
		else
		{
		pages.resize(stream->ReadIndex());
		for (FontPage& page : pages)
		{
			page.Texture = stream->ReadObject<UTexture>();
			page.Characters.resize(stream->ReadIndex());
			for (FontCharacter& character : page.Characters)
			{
				character.StartU = stream->ReadInt32();
				character.StartV = stream->ReadInt32();
				character.USize = stream->ReadInt32();
				character.VSize = stream->ReadInt32();
			}
		}

		}

		charactersPerPage = stream->ReadUInt32();
		if (engine->LaunchInfo.IsHarryPotter3() && stream->GetVersion() >= 120 && stream->GetVersion() < 122)
			ue2Kerning = stream->ReadInt32();

		if (stream->GetVersion() >= 69)
		{
			int count = stream->ReadIndex();
			for (int i = 0; i < count; i++)
			{
				uint16_t from = stream->ReadUInt16();
				uint16_t to = stream->ReadUInt16();
				charRemap[from] = to;
			}
			isRemapped = stream->ReadUInt32() != 0;
		}
	}
}

void UFont::Save(PackageStreamWriter* stream)
{
	if (stream->GetVersion() <= 63)
	{
		UTexture::Save(stream);

		FontPage& page = pages.front();
		stream->WriteIndex((int)page.Characters.size());
		for (FontCharacter& character : page.Characters)
		{
			stream->WriteInt32(character.StartU);
			stream->WriteInt32(character.StartV);
			stream->WriteInt32(character.USize);
			stream->WriteInt32(character.VSize);
		}
	}
	else
	{
		UObject::Save(stream);

		if (poaLayout)
		{
			stream->WriteIndex((int)poaCharacters.size());
			for (size_t i = 0; i < poaCharacters.size(); ++i)
			{
				const auto& c = poaCharacters[i];
				stream->WriteInt32(c.StartU); stream->WriteInt32(c.StartV);
				stream->WriteInt32(c.USize); stream->WriteInt32(c.VSize);
				stream->WriteUInt8(poaTextureIndices[i]);
			}
			stream->WriteIndex((int)pages.size());
			for (const auto& page : pages) stream->WriteObject(page.Texture);
		}
		else
		{
		stream->WriteIndex((int)pages.size());
		for (FontPage& page : pages)
		{
			stream->WriteObject(page.Texture);
			stream->WriteIndex((int)page.Characters.size());
			for (FontCharacter& character : page.Characters)
			{
				stream->WriteInt32(character.StartU);
				stream->WriteInt32(character.StartV);
				stream->WriteInt32(character.USize);
				stream->WriteInt32(character.VSize);
			}
		}

		}

		stream->WriteUInt32(charactersPerPage);
		if (engine->LaunchInfo.IsHarryPotter3() && stream->GetVersion() >= 120 && stream->GetVersion() < 122)
			stream->WriteInt32(ue2Kerning);

		if (stream->GetVersion() >= 69)
		{
			stream->WriteIndex((int)charRemap.size());
			for (auto& it : charRemap)
			{
				stream->WriteUInt16(it.first);
				stream->WriteUInt16(it.second);
			}
			stream->WriteUInt32(isRemapped ? 1 : 0);
		}
	}
}

FontGlyph (*UFont::ExtraGlyph)(const UFont* font, unsigned char c) = nullptr;
bool UFont::ByteText = false;

FontGlyph UFont::GetGlyph(uint32_t c) const
{
	if (ExtraGlyph && c < 256)
	{
		FontGlyph extra = ExtraGlyph(this, (unsigned char)c);
		if (extra.Texture)
			return extra;
	}
	FontGlyph glyph = FindGlyph(c);
	if (glyph.USize == 0)
	{
		if (c >= 'a' && c <= 'z')
		{
			c += 'A' - 'a';
			glyph = FindGlyph(c);
		}

		if (glyph.USize == 0)
			glyph = FindGlyph(32);
	}
	return glyph;
}

FontGlyph UFont::FindGlyph(uint32_t c) const
{
	size_t index = RemapChar(c);
	if (poaLayout)
	{
		if (index >= poaCharacters.size()) return {};
		const size_t atlas = poaTextureIndices[index];
		if (atlas >= pages.size()) return {};
		return { pages[atlas].Texture, poaCharacters[index] };
	}
	for (auto& page : pages)
	{
		if (index < page.Characters.size())
		{
			return { page.Texture, page.Characters[index] };
		}
		index -= page.Characters.size();
	}
	return {};
}

uint32_t UFont::RemapChar(uint32_t c) const
{
	if (!isRemapped)
		return c;
	auto it = c <= 0xffff ? charRemap.find((uint16_t)c) : charRemap.end();
	return it != charRemap.end() ? it->second : 32;
}

std::u32string UFont::Codepoints(const std::string& text)
{
	std::u32string result;
	if (ByteText)
	{
		// 8-bit text (HP1's and HP2's Russian in Windows-1251, src/hp1/HP1Cyrillic.cpp): a code point is a byte
		for (unsigned char ch : text)
			result.push_back(ch);
		return result;
	}
	UTF8Reader reader(text.data(), text.size());
	while (!reader.is_end())
	{
		result.push_back(reader.character());
		reader.next();
	}
	return result;
}

void UFont::SetNativeGlyphs(UTexture* texture, Array<FontCharacter> characters, std::unordered_map<uint16_t, uint16_t> remap)
{
	poaLayout = false;
	poaCharacters.clear();
	poaTextureIndices.clear();
	pages.resize(1);
	pages.front().Texture = texture;
	pages.front().Characters = std::move(characters);
	charactersPerPage = (int)pages.front().Characters.size();
	charRemap = std::move(remap);
	isRemapped = true;
	native = true;
}
