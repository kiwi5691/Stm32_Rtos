#include "../../SYSTEM/sys/sys.h"
#include "../../SYSTEM/delay/delay.h"
#include "../../SYSTEM/usart/usart.h"
#include "../LED/led.h"
#include "lcd.h"
/**
 * @brief       清屏函数
 * @param       color: 要清屏的颜色
 * @retval      无
 */
void lcd_clear1(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height;    /* 得到总点数 */
    lcd_set_cursor(0x00, 0x0000);   /* 设置光标位置 */
    lcd_write_ram_prepare();        /* 开始写入GRAM */

    for (index = 0; index < totalpoint; index++)
    {
        delay_ms(5);  /* 可以通过调节延时函数来观察LCD屏扫描方式 */
        LCD->LCD_RAM = color;
    }
}

int lcd_main_1(void)
{
    uint8_t x = 0;
    uint8_t lcd_id[12];
    HAL_Init();                                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);                 /* 设置时钟, 72Mhz */
    delay_init(72);                                     /* 延时初始化 */
    usart_init(115200);                                 /* 串口初始化为115200 */
    led_init();                                         /* 初始化LED */
    lcd_init();                                         /* 初始化LCD */
    g_point_color = RED;
    sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);  /* 将LCD ID打印到lcd_id数组 */

    lcd_scan_dir(L2R_U2D);  /* 扫描方向定义，默认是从左到右,从上到下，通过形参可以选择8种扫描方向 */

    while (1)
    {
        switch (x)
        {
            case 0:
                lcd_clear1(BLUE);
                break;

            case 1:
                lcd_clear1(RED);
                break;

            case 2:
                lcd_clear1(MAGENTA);
                break;

            case 3:
                lcd_clear1(GREEN);
                break;

            case 4:
                lcd_clear1(BRRED);
                break;
        }

        x++;

        if (x == 5)x = 0;

        LED0_TOGGLE();  //红灯闪烁
        delay_ms(100);
    }
}

