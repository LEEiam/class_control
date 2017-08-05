#include "gy-53.h"
#ifdef RT_USING_ANOP
#include "anop.h"
#endif
#ifdef RT_USING_FILTER
#include "kalman.h"

#define KALMAN_INIT_X   0
#define KALMAN_INIT_P   0
#define KALMAN_PMC      0.2
#define KALMAN_MEC      10
#endif

static rt_uint16_t raw_distance = 0;
static rt_uint16_t distance = 0;
static rt_device_t dev = RT_NULL;
static rt_mailbox_t mb = RT_NULL;
#ifdef RT_USING_FILTER
static kalman1_state filter;
#endif

static rt_err_t gy53_rx_ind(rt_device_t dev, rt_size_t size)
{
    return rt_mb_send(mb, size);
}

static void gy53_recv_thread_entry(void *parameter)
{
    rt_size_t size;
    static rt_uint8_t buffer[2 * sizeof(struct gy53_data)];
    static rt_size_t index = 0;
    
    while (1)
    {
        /* wait for new data */
        if (rt_mb_recv(mb, &size, RT_WAITING_FOREVER) == RT_EOK)
        {
            /* read new data and save it to buffer */
            rt_device_read(dev, 0, &buffer[index], size);
            
            index += size;
            if (index >= 2)
            {
                struct gy53_data *frame = (struct gy53_data *)buffer;
                
                /* find header */
                if (frame->head != GY53_DATA_HEADER)
                    index = 0;
                
                /* a full data frame has been received */
                if (index >= sizeof(struct gy53_data))
                {
                    switch (frame->type)
                    {
                        case GY53_DATA_TYPE:
                        {
                            rt_enter_critical();
                            
                            /* big endian to little endian */
                            raw_distance = ((frame->data & 0x00FF) << 8) | 
                                            ((frame->data & 0xFF00) >> 8);
                            #ifdef RT_USING_FILTER
                            distance = kalman1_filter(&filter, raw_distance);
                            #endif
                            rt_exit_critical();
                            #ifdef RT_USING_ANOP
                                anop_upload_short(ANOP_FUNC_CUSTOM_4, (short *)&raw_distance, 1);
                                #ifdef RT_USING_FILTER
                                anop_upload_short(ANOP_FUNC_CUSTOM_5, (short *)&distance, 1);
                                #endif
                            #endif
                        }break;
                        
                        default:
                            break;
                    }
                    index = 0;
                }
            }
        }
    }
}

int gy53_init(void)
{
    rt_thread_t tid;
    
    #ifdef RT_USING_FILTER
    kalman1_init(&filter, KALMAN_INIT_X, KALMAN_INIT_P, KALMAN_PMC, KALMAN_MEC);
    #endif
    
    dev = rt_device_find("uart2");
    if (dev == RT_NULL)
        return -RT_ERROR;
    
    mb = rt_mb_create("gy53_mb", 4, RT_IPC_FLAG_FIFO);
    if (mb == RT_NULL)
        return -RT_ENOMEM;
    
    tid = rt_thread_create("gy53", gy53_recv_thread_entry, RT_NULL, 
                            1024, RT_THREAD_PRIORITY_MAX / 2, 10);
    if (tid == RT_NULL)
        return -RT_ENOMEM;
    rt_thread_startup(tid);
    
    rt_device_set_rx_indicate(dev, gy53_rx_ind);
    
    return rt_device_open(dev, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
}
//INIT_COMPONENT_EXPORT(gy53_init);

rt_err_t gy53_write_cmd(rt_uint8_t cmd)
{
    struct gy53_cmd frame;
    
    frame.head = GY53_CMD_HEADER;
    frame.cmd = cmd;
    frame.sum = frame.head + frame.cmd;
    rt_device_write(dev, 0, &frame, sizeof(struct gy53_cmd));
    
    return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void cmd_gy53_write_cmd(int argc, char *argv[])
{
    if (argc >= 3)
    {
        int i;
        
        for (i = 1; i < argc; i++)
        {
            switch (argv[i++][1])
            {
                case 'o':
                case 'm':
                    gy53_write_cmd(strtol(argv[i], NULL, 16));break;
                case 's':
                    gy53_write_cmd(GY53_CMD_SAVE);break;
                default:
                    break;
            }
        }
    }
    else
    {
        rt_kprintf("Usage:  %s [-o output] [-m mode] [-s]\n", argv[0]);
        rt_kprintf("-o: 0x45    continuously\n");
        rt_kprintf("    0x15    once query\n");
        rt_kprintf("-m: 0x50    long range\n");
        rt_kprintf("    0x51    fast\n");
        rt_kprintf("    0x52    accurate\n");
        rt_kprintf("    0x53    general\n");
        rt_kprintf("-s:         save");
    }
}
MSH_CMD_EXPORT_ALIAS(cmd_gy53_write_cmd, gy53, operations to gy53);

void dump_distance(void)
{
    rt_kprintf("distance: %4dmm\n", 
#ifdef RT_USING_FILTER
    distance
#else
    raw_distance
#endif
    );
}
MSH_CMD_EXPORT_ALIAS(dump_distance, distance, dump distance from sensor);

#ifdef RT_USING_FILTER
void cmd_gy53_kalman(int argc, char *argv[])
{
    if (argc >= 3)
    {
        int i;
        
        for (i = 1; i < argc; i++)
        {
            switch (argv[i++][1])
            {
                case 'q':
                {
                    rt_enter_critical();
                    filter.q = atof(argv[i]);
                    rt_exit_critical();
                }break;
                case 'r':
                {
                    rt_enter_critical();
                    filter.r = atof(argv[i]);
                    rt_exit_critical();
                }break;
                default:
                    break;
            }
        }
    }
    else
        rt_kprintf("Usage:  %s [-q predict] [-r measure]\n", argv[0]);
}
MSH_CMD_EXPORT_ALIAS(cmd_gy53_kalman, gy53_kalman, config kalman args of gy53);
#endif
#endif /* RT_USING_FINSH */
