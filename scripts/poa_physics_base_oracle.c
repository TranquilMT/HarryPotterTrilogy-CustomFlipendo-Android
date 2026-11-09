/* Isolated EU physics setter reference runner. No Windows entry point or game code runs.
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


static u32 actor[0x500/4],counters[16],vtable[0xd0/4],baseActor[4];static int baseCalls,notifyFlag;static float normal[3];
struct Vector{float x,y,z;};
__attribute__((thiscall,noinline))static void findBase(void*self){(void)self;}
__attribute__((thiscall,noinline))static void setBase(void*self,void*base,struct Vector floor,int notify){
 ((u32*)self)[0x4c/4]=(u32)base;++baseCalls;notifyFlag=notify;normal[0]=floor.x;normal[1]=floor.y;normal[2]=floor.z;
}
typedef void(__attribute__((thiscall))*SetPhysics)(void*,u8,void*,struct Vector);
struct Record{int oldMode,newMode,hadBase,hasBase,calls,notify;float floor[3];};
void _start(void){
 const u8*pe=dll_start+U32(dll_start+60);const u8*opt=pe+24;u32 base=U32(opt+28),size=U32(opt+56);u8*image=map(base,size);
 if((u32)image!=base||setup_fs()<0){syscall1(1,101);for(;;){}}
 copy(image,dll_start,U32(opt+60));const u8*sections=opt+U16(pe+20);
 for(u32 i=0;i<U16(pe+6);++i){const u8*s=sections+i*40;copy(image+U32(s+12),dll_start+U32(s+20),U32(s+16));}
 *(u8*)0x10305646=0xe9;*(u32*)0x10305647=(u32)findBase-0x1030564b;
 *(u32*)0x10951edc=(u32)counters;*(u32*)0x10952184=0;vtable[0xcc/4]=(u32)setBase;
 for(int had=0;had<2;++had)for(int old=0;old<13;++old)for(int target=0;target<13;++target){
  for(u32 i=0;i<sizeof(actor)/4;++i)actor[i]=0;
  actor[0]=(u32)vtable;actor[0x4c/4]=had?(u32)baseActor:0;((u8*)actor)[0x41]=(u8)old;baseCalls=notifyFlag=0;normal[0]=normal[1]=normal[2]=0;
  struct Vector floor={0,0,1};((SetPhysics)0x1047f6e0)(actor,(u8)target,0,floor);
  struct Record r;r.oldMode=old;r.newMode=((u8*)actor)[0x41];r.hadBase=had;r.hasBase=actor[0x4c/4]!=0;r.calls=baseCalls;r.notify=notifyFlag;for(int i=0;i<3;++i)r.floor[i]=normal[i];
  if(write(1,&r,sizeof(r))!=sizeof(r)){syscall1(1,102);for(;;){}}
 }
 syscall1(1,0);for(;;){}
}
