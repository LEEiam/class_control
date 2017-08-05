#ifndef _GY_53_H_
#define _GY_53_H_

#include <rtthread.h>

__packed struct gy53_cmd
{
#define GY53_CMD_HEADER     0xA5
    rt_uint8_t head;
#define GY53_CMD_CONTI_OUT  0x45
#define GY53_CMD_QUERY_OUT  0x15
#define GY53_CMD_SAVE       0x25
#define GY53_CMD_MODE_LONG  0x50
#define GY53_CMD_MODE_FAST  0x51
#define GY53_CMD_MODE_ACCU  0x52
#define GY53_CMD_MODE_GEN   0x53
#define GY53_CMD_RATE_SLOW  0xAE
#define GY53_CMD_RATE_FAST  0xAF
    rt_uint8_t cmd;
    rt_uint8_t sum;
};

__packed struct gy53_data
{
#define GY53_DATA_HEADER    0x5A5A
    rt_uint16_t head;
#define GY53_DATA_TYPE      0x15
    rt_uint8_t  type;
#define GY53_DATA_AMOUNT    0x03
    rt_uint8_t  amnt;
    rt_uint16_t data;   //big endian
#define GY53_DATA_MODE_LONG 0x00
#define GY53_DATA_MODE_FAST 0x01
#define GY53_DATA_MODE_ACCU 0x02
#define GY53_DATA_MODE_GEN  0x03
    rt_uint8_t  mode;
    rt_uint8_t  sum;
};

rt_err_t gy53_write_cmd(rt_uint8_t cmd);

#endif  /* _GY_53_H_ */
