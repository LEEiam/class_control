#ifndef _DCMI_H_
#define _DCMI_H_

#include <rtthread.h>

void DCMI_DMA_Init(rt_ubase_t  DMA_Memory0BaseAddr, 
                   rt_uint16_t DMA_BufferSize, 
                   rt_size_t   DMA_MemoryDataSize);//DCMI��DMA����
void DCMI_Start(void);//����DCMI����
void DCMI_Stop(void);//�ر�DCMI����

#endif  /* _DCMI_H_ */
