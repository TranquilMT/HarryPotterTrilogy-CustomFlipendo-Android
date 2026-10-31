#pragma once

#include "Textures/UTexture.h"

class UTexture;

class FontCharacter
{
public:
	int StartU = 0;
	int StartV = 0;
	int USize = 0;
	int VSize = 0;
};

class FontPage
{
public:
	UTexture* Texture = nullptr;
	Array<FontCharacter> Characters;
};

class FontGlyph
{
public:
	FontGlyph() = default;
	FontGlyph(UTexture* texture, const FontCharacter& c) : Texture(texture), StartU(c.StartU), StartV(c.StartV), USize(c.USize), VSize(c.VSize) {}

	UTexture* Texture = nullptr;
	int StartU = 0;
	int StartV = 0;
	int USize = 0;
	int VSize = 0;
};

class UFont : public UTexture
{
public:
	using UTexture::UTexture;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	// c is a code point; engine strings are UTF-8, so walk them with Codepoints()
	FontGlyph GetGlyph(uint32_t c) const;

	static std::u32string Codepoints(const std::string& text);

	// Glyphs a font lacks, from elsewhere (HP1's Cyrillic text draws them from a TrueType font): consulted first when set, an
	// empty glyph (no texture) means the font's own. Only for code points below 256.
	static FontGlyph (*ExtraGlyph)(const UFont* font, unsigned char c);
	// Text is 8-bit (Windows-1251) instead of UTF-8: Codepoints() then takes every byte as a code point
	static bool ByteText;

	// A font made at runtime from a system font (HP1's Console.CreateNativeFont): one page holding the glyph slots,
	// and the code point -> slot map. Its text blends as anti-aliased coverage in the draw color, whatever the style.
	void SetNativeGlyphs(UTexture* texture, Array<FontCharacter> characters, std::unordered_map<uint16_t, uint16_t> remap);
	bool IsNative() const { return native; }

private:
	FontGlyph FindGlyph(uint32_t c) const;
	uint32_t RemapChar(uint32_t c) const;

	Array<FontPage> pages;
	int charactersPerPage;

	// Fonts from package version 69 on can map characters to other glyph slots (CharRemap, used when IsRemapped);
	// a character missing from the map draws as a space. HP1's own fonts leave it empty.
	std::unordered_map<uint16_t, uint16_t> charRemap;
	bool isRemapped = false;
	bool native = false;
};
