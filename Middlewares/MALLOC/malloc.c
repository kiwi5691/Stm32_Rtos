/**
 ****************************************************************************************************
 * @file        malloc.c
 * @brief       内存管理 驱动
 *
 ****************************************************************************************************
 */

#include "malloc.h"

#if !(__ARMCC_VERSION >= 6010050)   /* 不是AC6编译器，即使用AC5编译器时 */

//32位cpu为例，cpu在访问内存的时候是以一个字进行传输的（计算机中，一个机器字长就是cpu位数，32位cpu，四个字节为一个字），
// 那么也就是说cpu读取数据每次都是四个字节四个字节的读取，计算机最初是从地址0开始寻址，那么寻址也就是4个字节4个字节的地址依次往后找。
// 如果我们只需要读取一个字节的char型变量，那么计算机也是读取它所在的4个字节出来，然后提取出那个目标字节就行了，因为32位计算机每次都是4字节为操作单位。我们来思考这样一种情况
//(align(m)即这个地址值一定是m的整数倍。并且其占用的空间，即大小,也是m的整数倍，__attribute__有没有都无所谓，有的话就强行指定一下具体的地址罢了

//__attribute__((align(n))), 如果n大于此结构体中小大基本数据类型size,那么依据最大基本数据类型size对齐;否则，依据n进行对齐;

//注意：__attribute__ at  方法是把变量定义到指定的区域，而且这个变量支持 是算式 ->通过这个关键字，
// 指定 mem2mapbase 这个大数组的存放位置为 SRAM 上的空间，如果不加这个关键字修饰，MDK 会默认把这些变量定义到 STM32 的内部空间

//此芯片无外扩SRAM内存池，只能用内部SRAM

/* 内存池(64字节对齐) ->如果不加上这个限制，在 可能出现错误，所以一定要加上这个 */
static  __attribute__ ((__align(64))) uint8_t mem1base[MEM1_MAX_SIZE];                                     /* 内部SRAM内存池 */
static __attribute__ ((__align(64))) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));     /* 外扩SRAM内存池 */

/* 内存管理表 */
static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                      /* 内部SRAM内存池MAP */
static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000 + MEM2_MAX_SIZE)));      /* 外扩SRAM内存池MAP */

#else   /* 是AC6编译器，使用AC6编译器时 */

/* mem2base的地址：0x68000000，AC6不支持at宏定义表达式，修改为以下的宏定义表达式！ */


/* 内存池(64字节对齐) */
static __ALIGNED(64) uint8_t mem1base[MEM1_MAX_SIZE];                                                       /* 内部SRAM内存池 */
static __ALIGNED(64) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((section(".bss.ARM.__at_0X68000000")));  /* 外扩SRAM内存池 */

/* 内存管理表 */
static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                          /* 内部SRAM内存池MAP */
static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0X680F0C00")));     /* 外扩SRAM内存池MAP */

#endif

/* 内存管理参数 */
const uint32_t memtblsize[SRAMBANK] = {MEM1_ALLOC_TABLE_SIZE, MEM2_ALLOC_TABLE_SIZE};   /* 内存表大小 */
const uint32_t memblksize[SRAMBANK] = {MEM1_BLOCK_SIZE, MEM2_BLOCK_SIZE};               /* 内存分块大小 */
const uint32_t memsize[SRAMBANK]    = {MEM1_MAX_SIZE, MEM2_MAX_SIZE};                   /* 内存总大小 */

/* 内存管理控制器 */
struct _m_mallco_dev mallco_dev =
        {
                my_mem_init,                    /* 内存初始化 */
                my_mem_perused,                 /* 内存使用率 */
                mem1base, mem2base,             /* 内存池 */
                mem1mapbase, mem2mapbase,       /* 内存管理状态表 */
                0, 0,                           /* 内存管理未就绪 */
        };

/**
 * @brief       复制内存
 * @param       *des : 目的地址
 * @param       *src : 源地址
 * @param       n    : 需要复制的内存长度(字节为单位)
 * @retval      无
 */
void my_mem_copy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;

    while (n--)*xdes++ = *xsrc++;
}

/**
 * @brief       设置内存值
 * @param       *s    : 内存首地址
 * @param       c     : 要设置的值
 * @param       count : 需要设置的内存大小(字节为单位)
 * @retval      无
 */
void my_mem_set(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;

    while (count--){
        *xs++ = c;
    }
}

/**
 * @brief       内存管理初始化
 * @param       memx : 所属内存块
 * @retval      无
 */
void my_mem_init(uint8_t memx)
{
    uint8_t mttsize = sizeof(MT_TYPE);  /* 获取memmap数组的类型长度(uint16_t /uint32_t)*/
    my_mem_set(mallco_dev.memmap[memx], 0, memtblsize[memx]*mttsize); /* 从起始位地址， 乘16字节 遍历整个块表大小，每一位设置为0，内存状态表数据清零 */
    mallco_dev.memrdy[memx] = 1;        /* 内存管理初始化OK */
}

/**
 * @brief       获取内存使用率
 * @param       memx : 所属内存块
 * @retval      使用率(扩大了10倍,0~1000,代表0.0%~100.0%)
 */
uint16_t my_mem_perused(uint8_t memx)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < memtblsize[memx]; i++)
    {
        if (mallco_dev.memmap[memx][i])used++;
    }

    return (used * 1000) / (memtblsize[memx]);
}

/**
 * @brief       内存分配(内部调用)
 * @param       memx : 所属内存块
 * @param       size : 要分配的内存大小(字节)
 * @retval      内存偏移地址
 *   @arg       0 ~ 0XFFFFFFFE : 有效的内存偏移地址
 *   @arg       0XFFFFFFFF     : 无效的内存偏移地址
 */
static uint32_t my_mem_malloc(uint8_t memx, uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb;     /* 需要的内存块数 */
    uint32_t cmemb = 0; /* 连续空内存块数 */
    uint32_t i;

    if (!mallco_dev.memrdy[memx])
    {
        mallco_dev.init(memx);          /* 未初始化,先执行初始化 */
    }

    if (size == 0) return 0XFFFFFFFF;   /* 不需要分配 */

    nmemb = size / memblksize[memx];    /* 获取需要分配的连续内存块数 */

    if (size % memblksize[memx]) nmemb++;

    for (offset = memtblsize[memx] - 1; offset >= 0; offset--)  /* 搜索整个内存控制区 */
    {
        if (!mallco_dev.memmap[memx][offset])
        {
            cmemb++;            /* 连续空内存块数增加 */
        }
        else
        {
            cmemb = 0;          /* 连续内存块清零 */
        }

        if (cmemb == nmemb)     /* 找到了连续nmemb个空内存块 */
        {
            for (i = 0; i < nmemb; i++) /* 标注内存块非空 */
            {
                mallco_dev.memmap[memx][offset + i] = nmemb;
            }

            return (offset * memblksize[memx]); /* 返回偏移地址 */
        }
    }

    return 0XFFFFFFFF;  /* 未找到符合分配条件的内存块 */
}

/**
 * @brief       释放内存(内部调用)
 * @param       memx   : 所属内存块
 * @param       offset : 内存地址偏移
 * @retval      释放结果
 *   @arg       0, 释放成功;
 *   @arg       1, 释放失败;
 *   @arg       2, 超区域了(失败);
 */
static uint8_t my_mem_free(uint8_t memx, uint32_t offset)
{
    int i;

    if (!mallco_dev.memrdy[memx])   /* 未初始化,先执行初始化 */
    {
        mallco_dev.init(memx);
        return 1;                   /* 未初始化 */
    }

    if (offset < memsize[memx])     /* 偏移在内存池内. */
    {
        int index = offset / memblksize[memx];      /* 偏移所在内存块号码 */
        int nmemb = mallco_dev.memmap[memx][index]; /* 内存块数量 */

        for (i = 0; i < nmemb; i++)                 /* 内存块清零 */
        {
            mallco_dev.memmap[memx][index + i] = 0;
        }

        return 0;
    }
    else
    {
        return 2;   /* 偏移超区了. */
    }
}

/**
 * @brief       释放内存(外部调用)
 * @param       memx : 所属内存块
 * @param       ptr  : 内存首地址
 * @retval      无
 */
void myfree(uint8_t memx, void *ptr)
{
    uint32_t offset;

    if (ptr == NULL)return;     /* 地址为0. */

    offset = (uint32_t)ptr - (uint32_t)mallco_dev.membase[memx];
    my_mem_free(memx, offset);  /* 释放内存 */
}

/**
 * @brief       分配内存(外部调用)
 * @param       memx : 所属内存块
 * @param       size : 要分配的内存大小(字节)
 * @retval      分配到的内存首地址.
 */
void *mymalloc(uint8_t memx, uint32_t size)
{
    uint32_t offset;
    offset = my_mem_malloc(memx, size);

    if (offset == 0XFFFFFFFF)   /* 申请出错 */
    {
        return NULL;            /* 返回空(0) */
    }
    else    /* 申请没问题, 返回首地址 */
    {
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);
    }
}

/**
 * @brief       重新分配内存(外部调用)
 * @param       memx : 所属内存块
 * @param       *ptr : 旧内存首地址
 * @param       size : 要分配的内存大小(字节)
 * @retval      新分配到的内存首地址.
 */
void *myrealloc(uint8_t memx, void *ptr, uint32_t size)
{
    uint32_t offset;
    offset = my_mem_malloc(memx, size);

    if (offset == 0XFFFFFFFF)   /* 申请出错 */
    {
        return NULL;            /* 返回空(0) */
    }
    else    /* 申请没问题, 返回首地址 */
    {
        my_mem_copy((void *)((uint32_t)mallco_dev.membase[memx] + offset), ptr, size); /* 拷贝旧内存内容到新内存 */
        myfree(memx, ptr);  /* 释放旧内存 */
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);   /* 返回新内存首地址 */
    }
}












