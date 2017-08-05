#include <rtthread.h>
#include "ili9486l.h"

/* screen width and height in pixels */
static rt_uint16_t ili9486l_width  = 480;
static rt_uint16_t ili9486l_height = 320;

/* LCD is connected to the FSMC_Bank1_NOR/SRAM1 and NE1 is used as ship select signal */
/* RS <==> A16 */
#define LCD_REG         (*((volatile rt_uint16_t *) 0x60000000)) /* RS = 0 */
#define LCD_RAM         (*((volatile rt_uint16_t *) 0x60020000)) /* RS = 1 */

rt_inline void lcd_write_cmd(rt_uint16_t cmd)
{
    LCD_REG = cmd;
}

rt_inline void lcd_write_data(rt_uint16_t data)
{
    LCD_RAM = data;
}

rt_inline rt_uint16_t lcd_read_data(void)
{
    return LCD_RAM;
}

rt_inline void lcd_mdelay(rt_uint32_t ms)
{
    rt_thread_delay(ms);
}

/******************************************************************************
                            Static Functions
*******************************************************************************/
static void ili9486l_display_on(void)
{
    /* Display On */
    lcd_write_cmd(LCD_DISPLAY_ON);
}

static void ili9486l_display_off(void)
{
    /* Display Off */
    lcd_write_cmd(LCD_DISPLAY_OFF);
}

void ili9486l_set_display_window(rt_uint16_t x, rt_uint16_t y, 
                                rt_uint16_t width, rt_uint16_t height)
{
	rt_uint16_t end_x = x + width  - 1;
	rt_uint16_t end_y = y + height - 1;
		
	/* Colomn address set */
	lcd_write_cmd(LCD_COLUMN_ADDR);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);
	lcd_write_data(end_x >> 8);
	lcd_write_data(end_x & 0xFF);
	
	/* Page address set */
	lcd_write_cmd(LCD_PAGE_ADDR); 
	lcd_write_data(y >> 8);
	lcd_write_data(y & 0xFF);
	lcd_write_data(end_y >> 8);
	lcd_write_data(end_y & 0xFF);
}

void ili9486l_set_pixel(const char *pixel, int x, int y)
{
    rt_uint16_t p = *(rt_uint16_t *)pixel;
    
    /*Set X position*/
	lcd_write_cmd(LCD_COLUMN_ADDR);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);
	
	/*Set Y position*/
	lcd_write_cmd(LCD_PAGE_ADDR);
	lcd_write_data(y >> 8);
	lcd_write_data(y & 0xFF);
	lcd_write_data(y >> 8);
	lcd_write_data(y & 0xFF);
    
    lcd_write_cmd(LCD_GRAM);
    lcd_write_data(p);
}

void ili9486l_get_pixel(char *pixel, int x, int y)
{
    rt_uint16_t p;
    
    /*Set X position*/
	lcd_write_cmd(LCD_COLUMN_ADDR);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);
	
	/*Set Y position*/
	lcd_write_cmd(LCD_PAGE_ADDR);
	lcd_write_data(y >> 8);
	lcd_write_data(y & 0xFF);
	lcd_write_data(y >> 8);
	lcd_write_data(y & 0xFF);
    
    lcd_write_cmd(LCD_RAMRD);
    p = lcd_read_data();    //dummy read
    p = lcd_read_data();
    
    *(rt_uint16_t *)pixel = p;
}

void ili9486l_draw_hline(const char *pixel, int x1, int x2, int y)
{
    rt_uint16_t p = *(rt_uint16_t *)pixel;
    
    /* draw horizontal line */
    ili9486l_set_display_window(x1, y, x2 - x1, 1);
    lcd_write_cmd(LCD_GRAM);
    while (x1 <= x2)
    {
        lcd_write_data(p);
        x1++;
    }
}

void ili9486l_draw_vline(const char *pixel, int x, int y1, int y2)
{
#define MV  (rt_uint16_t)(1u << 5)
    
    rt_uint16_t mac_value = 0;
    rt_uint16_t p = *(rt_uint16_t *)pixel;
    
    /* Toggle the MV bit (address is updated in vertical writing direction) */
    lcd_write_cmd(LCD_RDDMADCTL);
    mac_value = lcd_read_data();    //dummy read
    mac_value = lcd_read_data();
    lcd_write_cmd(LCD_MAC);
    lcd_write_data(mac_value ^ MV);
    
    /* draw vertical line */
    ili9486l_set_display_window(y1, x, y2 - y1, 1);
    lcd_write_cmd(LCD_GRAM);
    while (y1 <= y2)
    {
        lcd_write_data(p);
        y1++;
    }
    
    /* Toggle the MV bit (address is updated in horizontal writing direction) */
    lcd_write_cmd(LCD_MAC);
    lcd_write_data(mac_value);
}


void ili9486l_draw_blit_line(const char *pixel, int x, int y, rt_size_t size)
{
    rt_uint16_t *ptr = (rt_uint16_t *)pixel;
    
    /* draw horizontal line */
    ili9486l_set_display_window(x, y, size, 1);
    lcd_write_cmd(LCD_GRAM);
    while (size)
    {
        lcd_write_data(*ptr++);
        size--;
    }
}

static struct rt_device_graphic_ops ili9486l_ops =
{
    ili9486l_set_pixel,
    ili9486l_get_pixel,
    ili9486l_draw_hline,
    ili9486l_draw_vline,
    ili9486l_draw_blit_line
};

static rt_err_t ili9486l_init(rt_device_t dev)
{
    /* Configure LCD */
    lcd_write_cmd(LCD_POWER1);
    lcd_write_data(0x0F);
    lcd_write_data(0x0F);

    lcd_write_cmd(LCD_POWER2);
    lcd_write_data(0x41);

    lcd_write_cmd(LCD_VCOM1);
    lcd_write_data(0x00);
    lcd_write_data(0x3A);

    lcd_write_cmd(LCD_FRMCTR1);
    lcd_write_data(0xD0);
    lcd_write_data(0x11);

    //	 lcd_write_cmd(LCD_INVTR);
    //	 lcd_write_data(0x00);

    lcd_write_cmd(LCD_DFC);
    lcd_write_data(0x02);
    lcd_write_data(0x22);
    //	 lcd_write_data(0x3B);

    lcd_write_cmd(LCD_ETMOD);
    lcd_write_data(0xC6);

    lcd_write_cmd(LCD_PGAMMA);
    lcd_write_data(0x00);
    lcd_write_data(0x0B);
    lcd_write_data(0x10);
    lcd_write_data(0x03);
    lcd_write_data(0x10);
    lcd_write_data(0x08);
    lcd_write_data(0x33);
    lcd_write_data(0x89);
    lcd_write_data(0x48);
    lcd_write_data(0x07);
    lcd_write_data(0x0E);
    lcd_write_data(0x0C);
    lcd_write_data(0x28);
    lcd_write_data(0x2D);
    lcd_write_data(0x0F);

    lcd_write_cmd(LCD_NGAMMA);
    lcd_write_data(0x00);
    lcd_write_data(0x12);
    lcd_write_data(0x17);
    lcd_write_data(0x03);
    lcd_write_data(0x11);
    lcd_write_data(0x08);
    lcd_write_data(0x37);
    lcd_write_data(0x67);
    lcd_write_data(0x4C);
    lcd_write_data(0x07);
    lcd_write_data(0x0F);
    lcd_write_data(0x0C);
    lcd_write_data(0x2F);
    lcd_write_data(0x34);
    lcd_write_data(0x0F);

    lcd_write_cmd(LCD_3GAMMA_EN);
    lcd_write_data(0x18);
    lcd_write_data(0xA3);
    lcd_write_data(0x12);
    lcd_write_data(0x02);
    lcd_write_data(0xB2);
    lcd_write_data(0x12);
    lcd_write_data(0xFF);
    lcd_write_data(0x10);
    lcd_write_data(0x00);

    lcd_write_cmd(LCD_PRC);
    lcd_write_data(0xA9);
    lcd_write_data(0x91);
    lcd_write_data(0x2D);
    lcd_write_data(0x0A);
    lcd_write_data(0x4F);

    lcd_write_cmd(0XF8);
    lcd_write_data(0x21);
    lcd_write_data(0x04);
    
    //0xC8: top left, vertical screen
    //0x68: top left, horizontal screen
    lcd_write_cmd(LCD_MAC);
    lcd_write_data(0x68);

    lcd_write_cmd(LCD_PIXEL_FORMAT);
    lcd_write_data(0x55);

    lcd_write_cmd(0xF9);
    lcd_write_data(0x00);
    lcd_write_data(0x08);

    lcd_write_cmd(0xF4);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x08);
    lcd_write_data(0x91);
    lcd_write_data(0x04);
    
    /* Colomn address set */
	lcd_write_cmd(LCD_COLUMN_ADDR);
	lcd_write_data(0 >> 8);
	lcd_write_data(0 & 0xFF);
	lcd_write_data((ili9486l_width - 1) >> 8);
	lcd_write_data((ili9486l_width - 1) & 0xFF);
	
	/* Page address set */
	lcd_write_cmd(LCD_PAGE_ADDR); 
	lcd_write_data(0 >> 8);
	lcd_write_data(0 & 0xFF);
	lcd_write_data((ili9486l_height - 1) >> 8);
	lcd_write_data((ili9486l_height - 1) & 0xFF);
    
    lcd_write_cmd(LCD_DINVON);
    lcd_write_cmd(LCD_SLEEP_OUT);
    lcd_mdelay(120);
    lcd_write_cmd(LCD_DISPLAY_ON);

    return RT_EOK;
}

static rt_err_t ili9486l_open(rt_device_t dev, rt_uint16_t oflag)
{
    ili9486l_display_on();
    
    return RT_EOK;
}

static rt_err_t ili9486l_close(rt_device_t dev)
{
    ili9486l_display_off();
    
    return RT_EOK;
}

static rt_err_t ili9486l_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch (cmd)
    {
    case RTGRAPHIC_CTRL_GET_INFO:
    {
        struct rt_device_graphic_info *info;

        info = (struct rt_device_graphic_info*) args;
        RT_ASSERT(info != RT_NULL);

        info->bits_per_pixel = 16;
        info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565P;
        info->framebuffer = RT_NULL;
        info->width = ili9486l_width;
        info->height = ili9486l_height;
    }
    break;

    case RTGRAPHIC_CTRL_RECT_UPDATE:
        /* nothong to be done */
        break;

    default:
        break;
    }
    
    return RT_EOK;
}

/******************************************************************************
                            Golbal Functions
*******************************************************************************/
int rt_lcd_init(void)
{
    static struct rt_device lcd_device;

    /* register lcd device */
    lcd_device.type    = RT_Device_Class_Graphic;
    lcd_device.init    = ili9486l_init;
    lcd_device.open    = ili9486l_open;
    lcd_device.close   = ili9486l_close;
    lcd_device.read    = RT_NULL;
    lcd_device.write   = RT_NULL;
    lcd_device.control = ili9486l_control;
    
    lcd_device.user_data = &ili9486l_ops;

    /* register graphic device driver */
    return rt_device_register(&lcd_device, "lcd",
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
INIT_DEVICE_EXPORT(rt_lcd_init);
