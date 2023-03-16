/**
 ****************************************************************************************************
 * @file        main.c
 * @brief       内存管理 实验
 ****************************************************************************************************
 */

#include "../../SYSTEM/sys/sys.h"
#include "../../SYSTEM/usart/usart.h"
#include "../../SYSTEM/delay/delay.h"
#include "../../BSP/LED/led.h"
#include "../../BSP/LCD/lcd.h"
#include "../../BSP/KEY/key.h"
//#include "../../BSP/SDRAM/sdram.h"
#include "../MALLOC/malloc.h"
#include "freertos_demo.h"

const char *SRAM_NAME_BUF[SRAMBANK] = {" SRAMIN  ", " SRAMCCM ", " SRAMEX  "};

int main(void)
{
    HAL_Init();                                 /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);        /* 设置时钟,180Mhz */
    delay_init(180);                            /* 延时初始化 */
    usart_init(115200);                         /* 串口初始化为115200 */
    led_init();                                 /* 初始化LED */
    key_init();                                 /* 初始化按键 */
//    sdram_init();                               /* SRAM初始化 */
    lcd_init();                                 /* 初始化LCD */

    my_mem_init(SRAMIN);                        /* 初始化内部内存池 */
    my_mem_init(SRAMEX);                        /* 初始化外部内存池 */
//    my_mem_init(SRAMCCM);                       /* 初始化CCM内存池 */

    lcd_show_string(30,  50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30,  70, 200, 16, 16, "MALLOC TEST", RED);
    lcd_show_string(30,  90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Malloc & WR & Show", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEY_UP:SRAMx KEY1:Free", RED);
    lcd_show_string(60, 160, 200, 16, 16, " SRAMIN ", BLUE);
    lcd_show_string(30, 176, 200, 16, 16, "SRAMIN   USED:", BLUE);
    lcd_show_string(30, 192, 200, 16, 16, "SRAMCCM  USED:", BLUE);
    lcd_show_string(30, 208, 200, 16, 16, "SRAMEX   USED:", BLUE);

    freertos_demo();
}
