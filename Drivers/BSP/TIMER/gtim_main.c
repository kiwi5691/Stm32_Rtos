// PWM输出主函数

#include "../../SYSTEM/sys/sys.h"
#include "../../SYSTEM/delay/delay.h"
#include "../../SYSTEM/usart/usart.h"
#include "../LED/led.h"
#include "gtim.h"
#include "../KEY/key.h"
#include "atim.h"

extern TIM_HandleTypeDef g_timx_pwm_chy_handle;     /* 定时器x句柄 */

int pwm_main(void)
{
    uint16_t ledrpwmval = 0;
    uint8_t dir = 1;

    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);     /* 设置时钟, 72Mhz */
    delay_init(72);                         /* 延时初始化 */
    usart_init(115200);                     /* 串口初始化为115200 */
    led_init();                             /* 初始化LED */
    gtim_timx_pwm_chy_init(500 - 1, 72 - 1);/* 1Mhz的计数频率,2Khz的PWM. */

    while (1)
    {
        delay_ms(10);

        if (dir)ledrpwmval++;               /* dir==1 ledrpwmval递增 */
        else ledrpwmval--;                  /* dir==0 ledrpwmval递减 */

        if (ledrpwmval > 300)dir = 0;       /* ledrpwmval到达300后，方向为递减 */
        if (ledrpwmval == 0)dir = 1;        /* ledrpwmval递减到0后，方向改为递增 */

        /* 修改比较值控制占空比 */
        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, ledrpwmval);

//        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, 200);  //固定200
//        __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, 200); 删除上面的，查看波形是否会一致
    }
}



//输入捕获实验

extern uint8_t  g_timxchy_cap_sta;  /* 输入捕获状态 */
extern uint16_t g_timxchy_cap_val;  /* 输入捕获值 */

int cap_main(void)
{
    uint32_t temp = 0;
    uint8_t t = 0;

    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);     /* 设置时钟, 72Mhz */
    delay_init(72);                         /* 延时初始化 */
    usart_init(115200);                     /* 串口初始化为115200 */
    led_init();                             /* 初始化LED */
    gtim_timx_cap_chy_init(0XFFFF, 72 - 1); /* 以1Mhz的频率计数 捕获 */
    //基本定时器讲的定时器溢出公式： Tout= ((arr+1)*(psc+1))/Tclk= ((499+1)*(71+1))/72000000=0.0005s
    while (1)
    {
        if (g_timxchy_cap_sta & 0X80)       /* 成功捕获到了一次高电平 */
        {
            //这里的含义是：捕获上沿的的电平中 加上 溢出的时间，
            temp = g_timxchy_cap_sta & 0X3F;
            temp *= 65536;                  /* 溢出时间总和 */
            temp += g_timxchy_cap_val;      /* 得到总的高电平时间 */
            printf("HIGH:%d us\r\n", temp); /* 打印总的高点平时间 */
            g_timxchy_cap_sta = 0;          /* 开启下一次捕获*/
        }

        t++;

        if (t > 20)         /* 200ms进入一次 */
        {
            t = 0;
            LED0_TOGGLE();  /* LED0闪烁 ,提示程序运行 */
        }
        delay_ms(10);
    }
}


//通用定时器脉冲计数实验程序

int cnt_chy_main(void)
{
    uint32_t curcnt = 0;
    uint32_t oldcnt = 0;
    uint8_t key = 0;
    uint8_t t = 0;

    HAL_Init();                                 /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);         /* 设置时钟, 72Mhz */
    delay_init(72);                             /* 延时初始化 */
    usart_init(115200);                         /* 串口初始化为115200 */
    led_init();                                 /* 初始化LED */
    key_init();                                 /* 初始化按键 */
    gtim_timx_cnt_chy_init(0);                  /* 定时器计数初始化, 不分频 */
    gtim_timx_cnt_chy_restart();                /* 重启计数 */

    while (1)
    {
        key = key_scan(0);                      /* 扫描按键 */

        if (key == KEY0_PRES)                   /* KEY0按键按下,重启计数 */
        {
            gtim_timx_cnt_chy_restart();        /* 重新启动计数 */
        }
//应该是else，感觉他写错了
        curcnt = gtim_timx_cnt_chy_get_count(); /* 获取计数值 */

        if (oldcnt != curcnt)
        {
            oldcnt = curcnt;
            printf("CNT:%d\r\n", oldcnt);       /* 打印脉冲个数 */
        }

        t++;

        if (t > 20)                             /* 200ms进入一次 */
        {
            t = 0;
            LED0_TOGGLE();                      /* LED0闪烁 ,提示程序运行 */
        }

        delay_ms(10);
    }
}

//PWM输入模式

extern uint16_t g_timxchy_pwmin_psc;    /* PWM输入状态 */
extern uint16_t g_timxchy_pwmin_sta;    /* PWM输入状态 */
extern uint32_t g_timxchy_pwmin_hval;   /* PWM的高电平脉宽 */
extern uint32_t g_timxchy_pwmin_cval;   /* PWM的周期宽度 */

int pwmin_main(void)
{
    uint8_t t = 0;
    double ht, ct, f, tpsc;

    HAL_Init();                                 /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);         /* 设置时钟, 72Mhz */
    delay_init(72);                             /* 延时初始化 */
    usart_init(115200);                         /* 串口初始化为115200 */
    led_init();                                 /* 初始化LED */
    //直接使用通用定时器的 PWM 输出，让 TIM3_CH2(PB5)输出 PWM 波
    //用杜邦线把 PB5 和 PC6 连接起来
    //PB5 输出的 PWM 就可以输入到 PC6（定时器 8 通道 1）
    gtim_timx_pwm_chy_init(10 - 1, 72 - 1);     /* 1Mhz的计数频率, 100Khz PWM */
    atim_timx_pwmin_chy_init();                 /* 初始化PWM输入捕获 */

    //PB5的PWM
    GTIM_TIMX_PWM_CHY_CCRX = 2;                 /* 低电平宽度20,高电平宽度80 */

    while (1)
    {
        //IC2下降沿->g_timxchy_pwmin_hval 算出宽度：即PWM上升开始到下降的宽度
        //IC1上升沿->g_timxchy_pwmin_cval 算出周期，PWM上升到结束，再到上升，为一个周期。
        delay_ms(10);
        t++;

        if (t >= 20)    /* 每200ms输出一次结果,并闪烁LED0,提示程序运行 */
        {
            if (g_timxchy_pwmin_sta)    /* 捕获了一次数据 */
            {
                printf("\r\n");                                     /* 输出空,另起一行 */
                printf("PWM PSC  :%d\r\n", g_timxchy_pwmin_psc);    /* 打印分频系数 */
                printf("PWM Hight:%d\r\n", g_timxchy_pwmin_hval);   /* 打印高电平脉宽 */
                printf("PWM Cycle:%d\r\n", g_timxchy_pwmin_cval);   /* 打印周期 */
                tpsc = ((double)g_timxchy_pwmin_psc + 1) / 72;      /* 得到PWM采样时钟周期时间 */
                ht = g_timxchy_pwmin_hval * tpsc;                   /* 计算高电平时间 */
                ct = g_timxchy_pwmin_cval * tpsc;                   /* 计算周期长度 */
                f = (1 / ct) * 1000000;                             /* 计算频率 */
                printf("PWM Hight time:%.3fus\r\n", ht);            /* 打印高电平脉宽长度 */
                printf("PWM Cycle time:%.3fus\r\n", ct);            /* 打印周期时间长度 */
                printf("PWM Frequency :%.3fHz\r\n", f);             /* 打印频率 */
                atim_timx_pwmin_chy_restart(); /* 重启PWM输入检测 */
            }

            LED1_TOGGLE();  /* LED1（GREEN）闪烁 */
            t = 0;
        }
    }
}