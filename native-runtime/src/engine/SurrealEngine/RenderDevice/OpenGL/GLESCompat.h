#pragma once

// OpenGL ES 3.2 in place of desktop OpenGL 4.2 (Android). The device is written against desktop GL: the few calls ES lacks
// are mapped here, and the states ES doesn't have are skipped where they are used (GLRenderDevice.cpp, under __ANDROID__).

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <cstring>

// No loader: libGLESv3 exports every ES 3.2 entry point.
#define ogl_LOAD_FAILED 0
#define ogl_LOAD_SUCCEEDED 1
inline int ogl_LoadFunctions() { return ogl_LOAD_SUCCEEDED; }

// Desktop enums the shared code names. They are only used on paths that check IsGLES() or that ES ignores.
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_RGBA16
#define GL_RGBA16 0x805B
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#endif
#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS 0x8501
#endif
#ifndef GL_MIRROR_CLAMP_TO_EDGE
#define GL_MIRROR_CLAMP_TO_EDGE 0x8743
#endif
#ifndef GL_DEPTH_CLAMP
#define GL_DEPTH_CLAMP 0x864F
#endif
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

inline void glClearDepth(double depth) { glClearDepthf((GLfloat)depth); }
inline void glDepthRange(double nearVal, double farVal) { glDepthRangef((GLfloat)nearVal, (GLfloat)farVal); }
inline void glDrawBuffer(GLenum buffer) { glDrawBuffers(1, &buffer); }

// ES 3.1 has the immutable multisample storage only (MSAA is off on Android, kept for completeness)
inline void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

// glFramebufferTexture is ES 3.2; every attachment here is a 2D texture
inline void glFramebufferTextureCompat(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texture, level);
}
#define glFramebufferTexture glFramebufferTextureCompat

// An extension in the context's list (GL_EXTENSIONS per index, ES 3.0)
inline bool GLHasExtension(const char* name)
{
	GLint count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &count);
	for (GLint i = 0; i < count; i++)
	{
		const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
		if (ext && strcmp(ext, name) == 0)
			return true;
	}
	return false;
}
