/* Isolated EU math reference runner. No Windows entry point or game code runs.
   Build as static i386 without a C runtime on a host with IA32 execution. */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
extern const u8 dll_start[];
__asm__(".section .rodata\n.global dll_start\ndll_start:\n.incbin \"recovery/Core.dll\"\n.text\n");
static u32 U32(const u8*p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
static u16 U16(const u8*p){return p[0]|((u16)p[1]<<8);}
static void copy(u8*d,const u8*s,u32 n){while(n--)*d++=*s++;}
static int syscall1(int n,int a){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a):"memory");return r;}
static int write(int fd,const void*p,u32 n){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(4),"b"(fd),"c"(p),"d"(n):"memory");return r;}
static void* map(u32 base,u32 size){u32 args[6]={base,size,7,0x32,(u32)-1,0};int r;__asm__ volatile("int $0x80":"=a"(r):"a"(90),"b"(args):"memory");return(void*)r;}
static float sine(double a){double s;__asm__("fsin":"=t"(s):"0"(a));return(float)s;}
/* MSVC _ftol consumes ST0 and returns a truncating signed 64-bit value. */
__attribute__((noinline)) static long long ftol(void){u16 old,mode;long long result;__asm__ volatile("fnstcw %0":"=m"(old));mode=old|0xc00;__asm__ volatile("fldcw %1;fistpll %0;fldcw %2":"=m"(result):"m"(mode),"m"(old):"memory");return result;}
struct Rot{int pitch,yaw,roll;};struct Quat{float x,y,z,w;};struct Coords{float origin[3],x[3],y[3],z[3];};
typedef void*(__attribute__((thiscall))*Divide)(struct Coords*,const struct Rot*);
typedef void*(__attribute__((thiscall))*ToQuat)(struct Quat*,struct Coords);
typedef void*(__attribute__((thiscall))*ToCoords)(struct Coords*,struct Quat);
typedef void*(__attribute__((thiscall))*ToRot)(struct Coords*,struct Rot*);
struct Record{struct Rot input;struct Quat quat;struct Rot output;struct Coords basis;};
void _start(void){
 const u8*pe=dll_start+U32(dll_start+60);const u8*opt=pe+24;u32 base=U32(opt+28),size=U32(opt+56);u8*image=map(base,size);
 if((u32)image!=base){syscall1(1,101);for(;;){}}
 copy(image,dll_start,U32(opt+60));const u8*sections=opt+U16(pe+20);
 for(u32 i=0;i<U16(pe+6);++i){const u8*s=sections+i*40;copy(image+U32(s+12),dll_start+U32(s+20),U32(s+16));}
 float*table=(float*)0x101a7b64;for(int i=0;i<16384;++i)table[i]=sine((double)i*6.2831853071795864769/16384.0);
 /* Patch only the isolated math helper's compiler conversion thunk. */
 u8*thunk=(u8*)0x10159950;thunk[0]=0xe9;*(u32*)(thunk+1)=(u32)ftol-(u32)thunk-5;
 const struct Coords identity={{0,0,0},{1,0,0},{0,1,0},{0,0,1}};
 *(struct Coords*)0x101a7af0=identity; /* GMath.UnitCoords is runtime-initialized BSS. */
 const int angles[]={0,1,3,4,8192,16384,24576,32768,49152,65535,-12345};
 const int count=sizeof(angles)/sizeof(angles[0]);
 for(int i=0;i<count;++i)for(int j=0;j<count;++j)for(int k=0;k<count;++k){
  struct Record r;struct Coords c=identity;r.input=(struct Rot){angles[i],angles[j],angles[k]};
  ((Divide)0x101071c0)(&c,&r.input);((ToQuat)0x10120e10)(&r.quat,c);((ToCoords)0x10120fd0)(&r.basis,r.quat);((ToRot)0x10133590)(&r.basis,&r.output);
  if(write(1,&r,sizeof(r))!=sizeof(r)){syscall1(1,102);for(;;){}}
 }
 syscall1(1,0);for(;;){}
}
