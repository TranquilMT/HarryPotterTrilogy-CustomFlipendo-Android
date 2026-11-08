/* Isolated EU math reference runner. No Windows entry point or game code runs.
   Build as static i386 without a C runtime on a host with IA32 execution. */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
extern const u8 dll_start[];
__asm__(".section .rodata\n.global dll_start\ndll_start:\n.incbin \"recovery/Engine.dll\"\n.text\n");
static u32 U32(const u8*p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
static u16 U16(const u8*p){return p[0]|((u16)p[1]<<8);}
static void copy(u8*d,const u8*s,u32 n){while(n--)*d++=*s++;}
static int syscall1(int n,int a){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a):"memory");return r;}
static int write(int fd,const void*p,u32 n){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(4),"b"(fd),"c"(p),"d"(n):"memory");return r;}
static void* map(u32 base,u32 size){u32 args[6]={base,size,7,0x32,(u32)-1,0};int r;__asm__ volatile("int $0x80":"=a"(r):"a"(90),"b"(args):"memory");return(void*)r;}

static u32 seh;
struct Desc{int entry;u32 base,limit,flags;};
static int setup_fs(void){struct Desc descriptor={-1,(u32)&seh,0xfffff,0x51};int r;__asm__ volatile("int $0x80":"=a"(r):"a"(243),"b"(&descriptor):"memory");if(r<0)return r;u16 selector=(descriptor.entry<<3)|3;__asm__ volatile("movw %0,%%fs"::"r"(selector):"memory");return 0;}
static float frame_count;
__attribute__((thiscall,noinline)) static void* named(void*self,int name){(void)self;(void)name;return &frame_count;}
__attribute__((thiscall,noinline)) static float frames(void*self,void*animation){(void)self;(void)animation;return frame_count;}
typedef float(__attribute__((thiscall))*Get)(void*,int);
typedef void(__attribute__((thiscall))*SetFrame)(void*,int,float,int);
typedef void(__attribute__((thiscall))*Set)(void*,int,float);
typedef int(__attribute__((thiscall))*Freeze)(void*,float,int);
static u32 object[0x108/4],channel[0x70/4],vtable[0x120/4];
struct Record{int frames,unit;float input,phase,frame,setTime,rate,freeze,afterRate,afterTween;};
void _start(void){
 const u8*pe=dll_start+U32(dll_start+60);const u8*opt=pe+24;u32 base=U32(opt+28),size=U32(opt+56);u8*image=map(base,size);
 if((u32)image!=base||setup_fs()<0){syscall1(1,101);for(;;){}}
 copy(image,dll_start,U32(opt+60));const u8*sections=opt+U16(pe+20);
 for(u32 i=0;i<U16(pe+6);++i){const u8*s=sections+i*40;copy(image+U32(s+12),dll_start+U32(s+20),U32(s+16));}
 object[0]=(u32)vtable;object[0x100/4]=(u32)channel;object[0x104/4]=1;
 vtable[0xb8/4]=(u32)named;vtable[0xc8/4]=(u32)frames;vtable[0x114/4]=0x104e8ee0;
 const int counts[]={1,2,4,15,60};const float values[]={-.5f,0,.25f,.5f,1,3,59};
 for(int i=0;i<5;++i)for(int unit=0;unit<2;++unit)for(int j=0;j<7;++j){
  frame_count=(float)counts[i];struct Record r;r.frames=counts[i];r.unit=unit;r.input=values[j];
  *(float*)&channel[0x20/4]=30.0f/frame_count;
  ((SetFrame)0x104e8ee0)(object,0,r.input,unit);
  r.phase=((Get)0x104e44c0)(object,0);r.frame=((Get)0x104e43b0)(object,0);
  ((Set)0x104e2140)(object,0,r.input);r.setTime=((Get)0x104e44c0)(object,0);
  ((Set)0x104e8d90)(object,0,1.25f);r.rate=((Get)0x104e4370)(object,0);
  *(float*)&channel[0x18/4]=99;
  ((Freeze)0x104e4040)(object,-r.input,0);r.freeze=((Get)0x104e44c0)(object,0);r.afterRate=((Get)0x104e4370)(object,0);r.afterTween=*(float*)&channel[0x18/4];
  if(write(1,&r,sizeof(r))!=sizeof(r)){syscall1(1,102);for(;;){}}
 }
 syscall1(1,0);for(;;){}
}
