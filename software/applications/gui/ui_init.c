#include <rtthread.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/driver.h>

int ui_init(void)
{
    rt_device_t device;

    device = rt_device_find("lcd");
    if (device == RT_NULL)
    {
        rt_kprintf("no graphic device in the system.\n");
        return -1;
    }

    /* open lcd device */
    rt_device_open(device, RT_DEVICE_OFLAG_RDWR);
    
    /* re-set graphic device */
    rtgui_graphic_set_device(device);

    return 0;
}
INIT_APP_EXPORT(ui_init);
