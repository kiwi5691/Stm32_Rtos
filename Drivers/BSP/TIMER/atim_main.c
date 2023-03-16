
#include <stm32f1xx_hal.h>
#include "../../SYSTEM/sys/sys.h"
#include "../../SYSTEM/delay/delay.h"
#include "../../SYSTEM/usart/usart.h"
#include "../LED/led.h"
#include "../KEY/key.h"
#include "atim.h"

int repeat_pwm_main(void)
{
    uint8_t key = 0;
    uint8_t t = 0;
    GPIO_InitTypeDef gpio_init_struct;

    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    led_init();                         /* 初始化LED */
    key_init();                         /* 初始化按键 */

    /* 将 LED1 引脚设置为输入模式, 避免和PC6冲突 */
    gpio_init_struct.Pin = LED1_GPIO_PIN;                   /* LED1引脚 */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;                /* 设置为输入 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(LED1_GPIO_PORT, &gpio_init_struct);       /* 初始化LED1引脚 */

    atim_timx_npwm_chy_init(5000 - 1, 7200 - 1);            /* 10Khz的计数频率,2hz的PWM频率. 5000个*/

    //占空比则由捕获/比较寄存器（TIMx_CCRx）的值决定
    ATIM_TIMX_NPWM_CHY_CCRX = 2500; /* 设置PWM占空比,50%,这样可以控制每一个PWM周期,LED1(BLUE)
                                     * 有一半时间是亮的,一半时间是灭的,LED1亮灭一次,表示一个PWM波
                                     */
    atim_timx_npwm_chy_set(5);      /* 输出5个PWM波(控制LED1(BLUE)闪烁5次) */

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)           /* KEY0按下 */
        {
            atim_timx_npwm_chy_set(5); /* 输出5个PWM波(控制TIM8_CH1, 即PC6输出5个脉冲) */
        }

//        t++;
//        delay_ms(10);
//
//        if (t > 50)                   /* 控制LED0闪烁, 提示程序运行状态 */
//        {
//            t = 0;
//            LED0_TOGGLE();
//        }
    }
}




int comp_main(void)
{
    uint8_t t = 0;

    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    led_init();                         /* 初始化LED */

    atim_timx_comp_pwm_init(1000 - 1, 72 - 1); /* 1Mhz的计数频率 1Khz的周期. */

    ATIM_TIMX_COMP_CH1_CCRX = 250 - 1;  /* 通道1 相位25% */
    ATIM_TIMX_COMP_CH2_CCRX = 500 - 1;  /* 通道2 相位50% */
    ATIM_TIMX_COMP_CH3_CCRX = 750 - 1;  /* 通道3 相位75% */
    ATIM_TIMX_COMP_CH4_CCRX = 1000 - 1; /* 通道4 相位100% */

    while (1)
    {
        delay_ms(10);
        t++;

        if (t >= 20)
        {
            LED0_TOGGLE(); /* LED0(RED)闪烁 */
            t = 0;
        }
    }
}


int bkr_main(void)
{
    uint8_t t = 0;

    HAL_Init();                                /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);        /* 设置时钟, 72Mhz */
    delay_init(72);                            /* 延时初始化 */
    usart_init(115200);                        /* 串口初始化为115200 */
    led_init();                                /* 初始化LED */
    atim_timx_cplm_pwm_init(1000 - 1, 72 - 1); /* 1Mhz的计数频率 1Khz的周期. */
    atim_timx_cplm_pwm_set(300, 100);          /* 占空比:7:3, 死区时间 100 * tDTS */

    while (1)
    {
        delay_ms(10);
        t++;

        if (t >= 20)
        {
            LED0_TOGGLE(); /* LED0(RED)闪烁 */
            t = 0;
        }
    }
}
