
#include <fstream>
#include "Precomp.h"
#include "GLUploadManager.h"
#include "GLRenderDevice.h"
#include "GLCachedTexture.h"
#include "Engine.h"
#include "HP1.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#ifdef __ANDROID__
#include <unistd.h>
#endif

GLUploadManager::GLUploadManager(GLRenderDevice* renderer) : renderer(renderer)
{
}

GLUploadManager::~GLUploadManager()
{
}

bool GLUploadManager::SupportsTextureFormat(TextureFormat Format) const
{
	return GLTextureUploader::GetUploader(Format);
}

#ifdef __ANDROID__
// Settings > Graphics fix: the choice is the first line of this file
static bool GraphicsFixIs(const char* wanted)
{
	std::ifstream file("/sdcard/FlipendoHP/.flipendo-gltest");
	std::string choice;
	std::getline(file, choice);
	while (!choice.empty() && (choice.back() == 13 || choice.back() == 32))
		choice.pop_back();
	return choice == wanted;
}
#endif

void GLUploadManager::UploadTexture(GLCachedTexture* tex, const TextureInfo& texinfo, bool masked)
{
	// Texture overrides (src/hp1/mods/TextureOverrides.cpp, HP1 and HP2): upload a replacement image instead of the
	// package's, as the Vulkan device does. The scene keeps using texinfo (USize/VSize) for UVs.
	TextureInfo overrideInfo;
	Array<UnrealMipmap> overrideMips;
	if (engine->LaunchInfo.IsKnowWonder() && HP1::LoadTextureOverride(texinfo.Texture, overrideInfo.Format, overrideMips))
	{
		overrideInfo.Texture = texinfo.Texture;
		overrideInfo.CacheID = texinfo.CacheID;
		overrideInfo.Mips = overrideMips.data();
		overrideInfo.NumMips = (int)overrideMips.size();
		overrideInfo.USize = overrideMips[0].Width;
		overrideInfo.VSize = overrideMips[0].Height;
	}
	const TextureInfo& Info = overrideMips.empty() ? texinfo : overrideInfo;

	int width = Info.USize;
	int height = Info.VSize;
	int mipcount = Info.NumMips;

	GLTextureUploader* uploader = GLTextureUploader::GetUploader(Info.Format);

	if ((uint32_t)Info.USize > 16384 || (uint32_t)Info.VSize > 16384 || !uploader)
	{
		width = 1;
		height = 1;
		mipcount = 1;
		uploader = nullptr;
	}

	GLint internalFormat = GL_RGBA8;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;
	if (uploader)
	{
		internalFormat = uploader->GetInternalformat();
		if (uploader->GetFormat() != 0)
		{
			format = uploader->GetFormat();
			type = uploader->GetType();
		}
	}

	// Base texture must use complete 4x4 compression blocks in Direct3D 11 or some drivers crash.
	// It is unclear if some OpenGL drivers have the same problem or not.
	// 
	// We fix this by creating 1 or 2 additional mipmap levels that are empty.

	int minSize = Info.Format == TextureFormat::BC1 || (Info.Format >= TextureFormat::BC2 && Info.Format <= TextureFormat::BC6H) ? 4 : 0;
#ifdef __ANDROID__
	// OpenGL ES: BC textures are decoded to RGBA8 on the CPU, so there are no 4x4 blocks to complete. Uploading a 4x4 block
	// into a 2x2 or 1x1 mip level (what the compressed path does) is GL_INVALID_VALUE for an uncompressed level.
	minSize = 0;
#endif

	if (!tex->Texture)
	{
		if (width < minSize || height < minSize)
		{
			if (width == 1 || height == 1)
			{
				width *= 4;
				height *= 4;
				mipcount += 2;
				tex->DummyMipmapCount = 2;
			}
			else
			{
				width *= 2;
				height *= 2;
				mipcount += 1;
				tex->DummyMipmapCount = 1;
			}
		}

		tex->Texture = std::make_shared<GLTexture2D>();
		glBindTexture(GL_TEXTURE_2D, tex->Texture->Handle);
#ifdef __ANDROID__
		// OpenGL ES: immutable storage with a mip chain that is complete by construction (levels down to 1x1 at most). The
		// mutable glTexImage2D-per-level texture the desktop device makes is mipmap-incomplete on some drivers (Mali-G52 r32:
		// every mipmapped texture, the level's walls and floors, sampled black while single-level ones were fine).
		{
			static const bool noMips = access("/sdcard/FlipendoHP/gl_nomip.txt", F_OK) == 0 || GraphicsFixIs("nomip"); // test aid (Settings > Graphics fix): no mip chains at all
			int possible = 1;
			for (int size = std::max(width, height); size > 1; size >>= 1)
				possible++;
			mipcount = std::max(1, std::min(mipcount, possible));
			if (noMips)
				mipcount = 1;
			glTexStorage2D(GL_TEXTURE_2D, mipcount, internalFormat, width, height);
			ThrowIfGLError("UploadTexture (glTexStorage2D) failed");
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipcount - 1);
		}
#else
		int mipwidth = width;
		int mipheight = height;
		for (int miplevel = 0; miplevel < mipcount; miplevel++)
		{
			glTexImage2D(GL_TEXTURE_2D, miplevel, internalFormat, mipwidth, mipheight, 0, format, type, nullptr);
			ThrowIfGLError("UploadTexture failed");
			mipwidth = std::max(mipwidth >> 1, 1);
			mipheight = std::max(mipheight >> 1, 1);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, std::max(mipcount - 1, 0));
#endif
	}

	if (uploader)
		UploadData(tex->Texture.get(), Info, masked, uploader, tex->DummyMipmapCount, minSize);
	else
		UploadWhite(tex->Texture.get());

	renderer->Stats.Uploads++;
}

void GLUploadManager::UploadTextureRect(GLCachedTexture* tex, const TextureInfo& Info, int x, int y, int w, int h)
{
#ifdef __ANDROID__
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif
	GLTextureUploader* uploader = GLTextureUploader::GetUploader(Info.Format);
	if (!uploader || Info.NumMips < 1 || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > Info.Mips[0].Width || y + h > Info.Mips[0].Height || Info.Mips[0].Data.empty())
		return;

	size_t pixelsSize = uploader->GetUploadSize(x, y, w, h);
	pixelsSize = (pixelsSize + 15) / 16 * 16; // memory alignment

	uint8_t* data = GetUploadBuffer(pixelsSize);
	uploader->UploadRect(data, &Info.Mips[0], x, y, w, h, Info.Palette, false);

	// To do: do we need this?
	// int pitch = uploader->GetUploadSize(0, 0, w, 1);
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / bytesPerPixel);

	glBindTexture(GL_TEXTURE_2D, tex->Texture->Handle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, uploader->GetFormat(), uploader->GetType(), data);
	ThrowIfGLError("UploadTextureRect failed");

	renderer->Stats.RectUploads++;
}

void GLUploadManager::UploadData(GLTexture2D* image, const TextureInfo& Info, bool masked, GLTextureUploader* uploader, int dummyMipmapCount, int minSize)
{
#ifdef __ANDROID__
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // the upload buffers are tightly packed rows (RGB565 with an odd width, ...)
	GLint levelsAllocated = 0; // the texture is immutable storage: levels past its chain are not uploaded
	glBindTexture(GL_TEXTURE_2D, image->Handle);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_LEVELS, &levelsAllocated);
#endif
	for (int level = 0; level < Info.NumMips; level++)
	{
#ifdef __ANDROID__
		if (level + dummyMipmapCount >= levelsAllocated)
			break;
#endif
		UnrealMipmap* Mip = &Info.Mips[level];
		if (!Mip->Data.empty())
		{
			uint32_t mipwidth = std::max(Mip->Width, minSize);
			uint32_t mipheight = std::max(Mip->Height, minSize);

			int mipsize = uploader->GetUploadSize(0, 0, mipwidth, mipheight);

			auto data = (uint32_t*)GetUploadBuffer(mipsize);
			uploader->UploadRect(data, Mip, 0, 0, mipwidth, mipheight, Info.Palette, masked);

			if (uploader->GetFormat() != 0)
			{
				glBindTexture(GL_TEXTURE_2D, image->Handle);
				glTexSubImage2D(GL_TEXTURE_2D, level + dummyMipmapCount, 0, 0, mipwidth, mipheight, uploader->GetFormat(), uploader->GetType(), data);
				ThrowIfGLError("UploadData failed");
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, image->Handle);
				glCompressedTexImage2D(GL_TEXTURE_2D, level + dummyMipmapCount, uploader->GetInternalformat(), mipwidth, mipheight, 0, mipsize, data);
				ThrowIfGLError("UploadData(compressed) failed");
			}
		}
	}
}

void GLUploadManager::UploadWhite(GLTexture2D* image)
{
	auto data = (uint32_t*)GetUploadBuffer(sizeof(uint32_t));
	data[0] = 0xffffffff;
	glBindTexture(GL_TEXTURE_2D, image->Handle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	ThrowIfGLError("UploadWhite failed");
}

uint8_t* GLUploadManager::GetUploadBuffer(size_t size)
{
	// Using uint32_t vector for dword alignment
	size = (size + 3) / 4;
	if (UploadBuffer.size() < size)
		UploadBuffer.resize(size);
	return (uint8_t*)UploadBuffer.data();
}
