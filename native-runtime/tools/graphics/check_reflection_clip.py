import ctypes as c
E=c.CDLL('libEGL.so.1');E.eglGetProcAddress.argtypes=[c.c_char_p];E.eglGetProcAddress.restype=c.c_void_p
F=lambda name,ret,args:c.CFUNCTYPE(ret,*args)(E.eglGetProcAddress(name.encode()))
get=F('eglGetPlatformDisplayEXT',c.c_void_p,[c.c_uint,c.c_void_p,c.POINTER(c.c_int)]);d=get(0x31DD,None,None)
E.eglInitialize.argtypes=[c.c_void_p,c.POINTER(c.c_int),c.POINTER(c.c_int)];a=c.c_int();b=c.c_int();assert E.eglInitialize(d,c.byref(a),c.byref(b));print(a.value,b.value)
E.eglBindAPI(0x30A0);attrs=(c.c_int*13)(0x3033,1,0x3040,0x40,0x3024,8,0x3023,8,0x3022,8,0x3021,8,0x3038);config=c.c_void_p();n=c.c_int();E.eglChooseConfig.argtypes=[c.c_void_p,c.POINTER(c.c_int),c.POINTER(c.c_void_p),c.c_int,c.POINTER(c.c_int)];assert E.eglChooseConfig(d,attrs,c.byref(config),1,c.byref(n))and n.value
E.eglCreateContext.argtypes=[c.c_void_p,c.c_void_p,c.c_void_p,c.POINTER(c.c_int)];E.eglCreateContext.restype=c.c_void_p;ctx=E.eglCreateContext(d,config,None,(c.c_int*3)(0x3098,3,0x3038));assert ctx
E.eglCreatePbufferSurface.argtypes=[c.c_void_p,c.c_void_p,c.POINTER(c.c_int)];E.eglCreatePbufferSurface.restype=c.c_void_p;s=E.eglCreatePbufferSurface(d,config,(c.c_int*5)(0x3057,8,0x3056,8,0x3038));E.eglMakeCurrent.argtypes=[c.c_void_p,c.c_void_p,c.c_void_p,c.c_void_p];assert E.eglMakeCurrent(d,s,s,ctx);print(F('glGetString',c.c_char_p,[c.c_uint])(0x1F02))
# Compile the actual shader resources and Android converter, then exercise pixels.
import pathlib,subprocess,tempfile,sys
ROOT=pathlib.Path(__file__).resolve().parents[2];engine=ROOT/'src/engine/SurrealEngine';sys.path.insert(0,str(ROOT/'tools/poa'))
from test_core_seven_runtime import definition
resource=(engine/'RenderDevice/OpenGL/GLFileResource.cpp').read_text().replace('#include "Precomp.h"','')
converter=definition((engine/'RenderDevice/OpenGL/GLRenderDevice.cpp').read_text(),'static std::string ToGlslEs300(const std::string& source, bool vertex)\n{')
with tempfile.TemporaryDirectory()as tmp:
 p=pathlib.Path(tmp);(p/'shaders.cpp').write_text('#include <iostream>\n'+resource+'\n'+converter+'\nint main(){std::cout<<ToGlslEs300(GLFileResource::readAllText("shaders/Scene.vert"),true)<<char(0)<<ToGlslEs300(GLFileResource::readAllText("shaders/Scene.frag"),false);}')
 subprocess.run(['g++','-std=c++20','-I'+str(engine/'RenderDevice/OpenGL'),str(p/'shaders.cpp'),'-o',str(p/'shaders')],check=True)
 vert,frag=subprocess.check_output([str(p/'shaders')]).split(b'\0')
prefix=b'#version 300 es\nprecision highp float;\nprecision highp int;\nprecision highp sampler2D;\nlayout(std140) uniform;\n'
def compile_shader(kind,source):
 shader=F('glCreateShader',c.c_uint,[c.c_uint])(kind);src=c.c_char_p(prefix+source)
 F('glShaderSource',None,[c.c_uint,c.c_int,c.POINTER(c.c_char_p),c.c_void_p])(shader,1,c.byref(src),None);F('glCompileShader',None,[c.c_uint])(shader)
 status=c.c_int();F('glGetShaderiv',None,[c.c_uint,c.c_uint,c.POINTER(c.c_int)])(shader,0x8B81,c.byref(status))
 if not status.value:
  log=c.create_string_buffer(16384);F('glGetShaderInfoLog',None,[c.c_uint,c.c_int,c.c_void_p,c.c_void_p])(shader,len(log),None,log);raise RuntimeError(log.value.decode())
 return shader
program=F('glCreateProgram',c.c_uint,[])()
for shader in (compile_shader(0x8B31,vert),compile_shader(0x8B30,frag)):F('glAttachShader',None,[c.c_uint,c.c_uint])(program,shader)
F('glLinkProgram',None,[c.c_uint])(program);status=c.c_int();F('glGetProgramiv',None,[c.c_uint,c.c_uint,c.POINTER(c.c_int)])(program,0x8B82,c.byref(status))
if not status.value:
 log=c.create_string_buffer(16384);F('glGetProgramInfoLog',None,[c.c_uint,c.c_int,c.c_void_p,c.c_void_p])(program,len(log),None,log);raise RuntimeError(log.value.decode())
F('glUseProgram',None,[c.c_uint])(program)
# UBO: two matrices, plane, then hit index and std140 padding.
import struct
identity=[1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
ub=c.c_uint();F('glGenBuffers',None,[c.c_int,c.POINTER(c.c_uint)])(1,c.byref(ub));F('glBindBuffer',None,[c.c_uint,c.c_uint])(0x8A11,ub)
F('glBindBufferBase',None,[c.c_uint,c.c_uint,c.c_uint])(0x8A11,0,ub)
index=F('glGetUniformBlockIndex',c.c_uint,[c.c_uint,c.c_char_p])(program,b'ScenePushConstants');F('glUniformBlockBinding',None,[c.c_uint,c.c_uint,c.c_uint])(program,index,0)
va=c.c_uint();F('glGenVertexArrays',None,[c.c_int,c.POINTER(c.c_uint)])(1,c.byref(va));F('glBindVertexArray',None,[c.c_uint])(va)
vb=c.c_uint();F('glGenBuffers',None,[c.c_int,c.POINTER(c.c_uint)])(1,c.byref(vb));F('glBindBuffer',None,[c.c_uint,c.c_uint])(0x8892,vb)
vertices=(c.c_float*9)(-1,-1,0,3,-1,0,-1,3,0);F('glBufferData',None,[c.c_uint,c.c_ssize_t,c.c_void_p,c.c_uint])(0x8892,c.sizeof(vertices),vertices,0x88E4)
F('glEnableVertexAttribArray',None,[c.c_uint])(1);F('glVertexAttribPointer',None,[c.c_uint,c.c_int,c.c_uint,c.c_ubyte,c.c_int,c.c_void_p])(1,3,0x1406,0,0,None)
F('glVertexAttribI4ui',None,[c.c_uint,c.c_uint,c.c_uint,c.c_uint,c.c_uint])(0,0,0,0,0);F('glVertexAttrib4f',None,[c.c_uint,*[c.c_float]*4])(6,1,1,1,1)
texture=c.c_uint();F('glGenTextures',None,[c.c_int,c.POINTER(c.c_uint)])(1,c.byref(texture));F('glBindTexture',None,[c.c_uint,c.c_uint])(0x0DE1,texture);white=(c.c_ubyte*4)(255,255,255,255)
F('glTexImage2D',None,[c.c_uint,c.c_int,c.c_int,c.c_int,c.c_int,c.c_int,c.c_uint,c.c_uint,c.c_void_p])(0x0DE1,0,0x1908,1,1,0,0x1908,0x1401,white)
F('glTexParameteri',None,[c.c_uint,c.c_uint,c.c_int])(0x0DE1,0x2801,0x2600);F('glTexParameteri',None,[c.c_uint,c.c_uint,c.c_int])(0x0DE1,0x2800,0x2600)
F('glViewport',None,[c.c_int]*4)(0,0,8,8)
for plane,expected in [((1,0,0,0),32),((-1,0,0,0),32),((0,0,0,-1),0),((0,0,0,1),64),((0,0,0,0),64)]:
 values=struct.pack('36f4I',*(identity+identity+list(plane)),0,0,0,0);data=c.create_string_buffer(values);F('glBindBuffer',None,[c.c_uint,c.c_uint])(0x8A11,ub);F('glBufferData',None,[c.c_uint,c.c_ssize_t,c.c_void_p,c.c_uint])(0x8A11,len(values),data,0x88E8)
 F('glClearColor',None,[c.c_float]*4)(0,0,0,1);F('glClear',None,[c.c_uint])(0x4000);F('glDrawArrays',None,[c.c_uint,c.c_int,c.c_int])(4,0,3)
 pixels=(c.c_ubyte*256)();F('glReadPixels',None,[c.c_int,c.c_int,c.c_int,c.c_int,c.c_uint,c.c_uint,c.c_void_p])(0,0,8,8,0x1908,0x1401,pixels)
 error=F('glGetError',c.c_uint,[])();assert error==0,hex(error)
 lit=sum(pixels[i]>128 for i in range(0,256,4));assert lit==expected,(plane,lit,expected)
print('PASS: production GLES scene shaders link; 5 clipping pixel cases, including disabled clipping for menu/2D rendering')
