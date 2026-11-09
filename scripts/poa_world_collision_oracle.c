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


static u32 first[0x400/4],second[0x400/4];
typedef int(__attribute__((thiscall))*Blocked)(void*,void*);
struct Record{u32 selfMask,otherMask;int blocked;};
void _start(void){
 const u8*pe=dll_start+U32(dll_start+60);const u8*opt=pe+24;u32 base=U32(opt+28),size=U32(opt+56);u8*image=map(base,size);
 if((u32)image!=base){syscall1(1,101);for(;;){}}
 copy(image,dll_start,U32(opt+60));const u8*sections=opt+U16(pe+20);
 for(u32 i=0;i<U16(pe+6);++i){const u8*s=sections+i*40;copy(image+U32(s+12),dll_start+U32(s+20),U32(s+16));}
 second[0x70/4]=0x08000000;
 for(u32 flags=0;flags<256;++flags)for(u32 variant=0;variant<2;++variant){
  struct Record r;r.selfMask=flags;r.otherMask=variant?255:(flags^165);first[0x2ec/4]=r.selfMask;second[0x2ec/4]=r.otherMask;
  r.blocked=((Blocked)0x103e2910)(first,second);
  if(write(1,&r,sizeof(r))!=sizeof(r)){syscall1(1,102);for(;;){}}
 }
 syscall1(1,0);for(;;){}
}
