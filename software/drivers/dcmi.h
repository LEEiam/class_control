#ifndef _DCMI_H_
#define _DCMI_H_

#include <rtthread.h>

void DCMI_DMA_Init(rt_ubase_t  DMA_Memory0BaseAddr, 
                   rt_uint16_t DMA_BufferSize, 
                   rt_size_t   DMA_MemoryDataSize);//DCMI的DMA配置
void DCMI_Start(void);//启动DCMI传输
void DCMI_Stop(void);//关闭DCMI传输

#endif  /* _DCMI_H_ */
