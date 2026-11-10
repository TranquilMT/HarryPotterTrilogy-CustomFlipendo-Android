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

struct Plane{float x,y,z,w;};
struct Quad{float x,y,w,h,u,v,ul,vl,z;struct Plane color;};
struct Record{int id,sw,sh,count;float width,height,afterX,afterY;struct Quad quads[16];};
static struct Record record;
static int sw,sh;
static u32 object[128],material[32],canvasTable[64],materialTable[64];
__attribute__((thiscall,noinline))static int width(void*self){(void)self;return sw;}
__attribute__((thiscall,noinline))static int height(void*self){(void)self;return sh;}
__attribute__((thiscall,noinline))static struct Plane*color(void*self,struct Plane*out){(void)self;out->x=out->y=out->z=out->w=1;return out;}
__attribute__((noinline))static int floor_float(float value){int i=(int)value;return (float)i>value?i-1:i;}
__attribute__((thiscall,noinline))static void draw(void*self,void*mat,float x,float y,float w,float h,float u,float v,float ul,float vl,float z,struct Plane col,struct Plane fog){
 (void)self;(void)mat;(void)fog;
 if(record.count>=16){syscall1(1,105);for(;;){}}
 struct Quad*q=&record.quads[record.count++];q->x=x;q->y=y;q->w=w;q->h=h;q->u=u;q->v=v;q->ul=ul;q->vl=vl;q->z=z;q->color=col;
}
static void jump(u32 address,u32 target){u8*p=(u8*)address;p[0]=0xe9;*(u32*)(p+1)=target-address-5;}
typedef void(__attribute__((thiscall))*Stretch)(void*,void*,float,float,float,float);
void _start(void){
 const u8*pe=dll_start+U32(dll_start+60);const u8*opt=pe+24;u32 base=U32(opt+28),size=U32(opt+56);u8*image=map(base,size);
 if((u32)image!=base||setup_fs()<0){syscall1(1,101);for(;;){}}
 copy(image,dll_start,U32(opt+60));const u8*sections=opt+U16(pe+20);
 for(u32 i=0;i<U16(pe+6);++i){const u8*s=sections+i*40;copy(image+U32(s+12),dll_start+U32(s+20),U32(s+16));}
 jump(0x10303189,(u32)color);*(u32*)0x109653ec=(u32)floor_float;
 object[0]=(u32)canvasTable;material[0]=(u32)materialTable;canvasTable[0x74/4]=(u32)draw;materialTable[0x7c/4]=(u32)width;materialTable[0x80/4]=(u32)height;
 const int widths[]={16,17,32},heights[]={8,13,16};const float scales[]={.25f,.5f,1,1.25f,2};int id=0;
 for(int a=0;a<3;++a)for(int b=0;b<3;++b)for(int x=0;x<5;++x)for(int y=0;y<5;++y){
  sw=widths[a];sh=heights[b];for(u32 n=0;n<sizeof(record);++n)((u8*)&record)[n]=0;
  record.id=id++;record.sw=sw;record.sh=sh;record.width=sw*scales[x];record.height=sh*scales[y];
  ((u8*)object)[0x54]=1;*(float*)((u8*)object+0x48)=99;*(float*)((u8*)object+0x4c)=88;
  ((Stretch)0x103f6e70)(object,material,10,20,record.width,record.height);
  record.afterX=*(float*)((u8*)object+0x48);record.afterY=*(float*)((u8*)object+0x4c);
  if(write(1,&record,sizeof(record))!=sizeof(record)){syscall1(1,102);for(;;){}}
 }
 syscall1(1,0);for(;;){}
}
