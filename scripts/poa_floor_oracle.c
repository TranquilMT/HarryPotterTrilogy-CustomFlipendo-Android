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


static u32 actor[0x400/4],level[4],avt[0xd0/4],lvt[0xd8/4],bases[2][4];static int selected,baseCalls,notifyFlag;static float from[3],to[3],extent[3],normal[3];static u32 traceFlags;
struct Vector{float x,y,z;};
__attribute__((thiscall,noinline))static int trace(void*self,void*hit,void*source,const struct Vector*end,const struct Vector*start,u32 flags,struct Vector box){
 (void)self;if(source!=actor)return 0;from[0]=start->x;from[1]=start->y;from[2]=start->z;to[0]=end->x;to[1]=end->y;to[2]=end->z;extent[0]=box.x;extent[1]=box.y;extent[2]=box.z;traceFlags=flags;
 ((u32*)hit)[1]=selected?(u32)bases[selected-1]:0;((float*)hit)[5]=0;((float*)hit)[6]=0;((float*)hit)[7]=selected?1:0;return selected?0:1;
}
__attribute__((thiscall,noinline))static void setBase(void*self,void*base,struct Vector floor,int notify){
 ((u32*)self)[0x4c/4]=(u32)base;++baseCalls;notifyFlag=notify;normal[0]=floor.x;normal[1]=floor.y;normal[2]=floor.z;
}
typedef void(__attribute__((thiscall))*FindBase)(void*);
struct Record{u32 scenario,oldBase,hit;float from[3],to[3],extent[3];u32 flags,calls,newBase,notify;float normal[3];};
void _start(void){
 const u8*pe=dll_start+U32(dll_start+60);const u8*opt=pe+24;u32 base=U32(opt+28),size=U32(opt+56);u8*image=map(base,size);
 if((u32)image!=base||setup_fs()<0){syscall1(1,101);for(;;){}}
 copy(image,dll_start,U32(opt+60));const u8*sections=opt+U16(pe+20);
 for(u32 i=0;i<U16(pe+6);++i){const u8*s=sections+i*40;copy(image+U32(s+12),dll_start+U32(s+20),U32(s+16));}
 avt[0xcc/4]=(u32)setBase;lvt[0xd4/4]=(u32)trace;level[0]=(u32)lvt;
 for(u32 scenario=0;scenario<4;++scenario)for(u32 old=0;old<3;++old)for(u32 target=0;target<3;++target){
  for(u32 i=0;i<sizeof(actor)/4;++i)actor[i]=0;
  actor[0]=(u32)avt;actor[0xa0/4]=(u32)level;actor[0x4c/4]=old?(u32)bases[old-1]:0;
  ((float*)actor)[0x150/4]=10;((float*)actor)[0x154/4]=20;((float*)actor)[0x158/4]=30;
  ((float*)actor)[0x2dc/4]=2;((float*)actor)[0x2e0/4]=(scenario&1)?3:0;((float*)actor)[0x2e4/4]=5;((u8*)actor)[0x2e8]=(scenario>=2);
  selected=target;baseCalls=notifyFlag=0;normal[0]=normal[1]=normal[2]=0;
  ((FindBase)0x1047b380)(actor);
  struct Record r;r.scenario=scenario;r.oldBase=old;r.hit=target;r.flags=traceFlags;r.calls=baseCalls;r.notify=notifyFlag;r.newBase=actor[0x4c/4]==0?0:(actor[0x4c/4]==(u32)bases[0]?1:2);
  for(int i=0;i<3;++i){r.from[i]=from[i];r.to[i]=to[i];r.extent[i]=extent[i];r.normal[i]=normal[i];}
  if(write(1,&r,sizeof(r))!=sizeof(r)){syscall1(1,102);for(;;){}}
 }
 syscall1(1,0);for(;;){}
}
