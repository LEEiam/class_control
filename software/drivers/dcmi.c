#include "board.h"
#include "dcmi.h"

//DCMI�ӿڳ�ʼ��
int stm32_hw_dcmi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	DCMI_InitTypeDef DCMI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);//ʹ��DCMIʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|
                           RCC_AHB1Periph_GPIOB|
                           RCC_AHB1Periph_GPIOC|
                           RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOA B C E ʱ��
	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //���ù������
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    //GPIOA 4, 6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);//GPIOA��ʼ��
    //GPIOB 6, 7, 8
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);//GPIOB��ʼ��
    //GPIOC 6, 7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);//GPIOC��ʼ��
    //GPIOE 0, 1, 4, 6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_6;
    GPIO_Init(GPIOE, &GPIO_InitStructure);//GPIOE��ʼ��	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_DCMI); //PA4,AF13  DCMI_HSYNC
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_DCMI); //PA6,AF13  DCMI_PCLK  
 	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_DCMI); //PB7,AF13  DCMI_VSYNC 
 	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_DCMI); //PC6,AF13  DCMI_D0  
 	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_DCMI); //PC7,AF13  DCMI_D1 
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource0,GPIO_AF_DCMI); //PE0,AF13  DCMI_D2
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource1,GPIO_AF_DCMI); //PE1,AF13  DCMI_D3
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource4,GPIO_AF_DCMI); //PE4,AF13  DCMI_D4 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_DCMI); //PB6,AF13  DCMI_D5 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_DCMI); //PB8,AF13  DCMI_D6
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource6,GPIO_AF_DCMI); //PE6,AF13  DCMI_D7
	
	DCMI_DeInit();//���ԭ��������
	
	DCMI_InitStructure.DCMI_CaptureMode=DCMI_CaptureMode_Continuous;//����ģʽ
	DCMI_InitStructure.DCMI_CaptureRate=DCMI_CaptureRate_All_Frame;//ȫ֡����
	DCMI_InitStructure.DCMI_ExtendedDataMode=DCMI_ExtendedDataMode_8b;//8λ���ݸ�ʽ
	DCMI_InitStructure.DCMI_HSPolarity=DCMI_HSPolarity_Low;//HSYNC �͵�ƽ��Ч
	DCMI_InitStructure.DCMI_PCKPolarity=DCMI_PCKPolarity_Rising;//PCLK ��������Ч
	DCMI_InitStructure.DCMI_SynchroMode=DCMI_SynchroMode_Hardware;//Ӳ��ͬ��HSYNC,VSYNC
	DCMI_InitStructure.DCMI_VSPolarity=DCMI_VSPolarity_Low;//VSYNC �͵�ƽ��Ч
	DCMI_Init(&DCMI_InitStructure);
	
	DCMI_ITConfig(DCMI_IT_FRAME,ENABLE);//����֡�ж�
	NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	DCMI_Cmd(ENABLE);//DCMIʹ��
    
    return 0;
}
INIT_BOARD_EXPORT(stm32_hw_dcmi_init);

//DCMI֡�жϷ�����
#include <math.h>
#include "colortrace.h"
#include "dmp.h"

#define ORIGIN_XPIXEL       258         /*  */
#define ORIGIN_YPIXEL       148         /*  */
#define XPIXEL_TO_MM        1 / 1.75f   /*  */
#define YPIXEL_TO_MM        1 / 1.60f   /*  */
#define OFFSET_HEIGHT       36          /* offset height */
#define PI                  3.14159f    /* �� value in float type */
#define ANGLE_TO_RADIAN     PI / 180    /* convert angle to radian */

void DCMI_IRQHandler(void)
{
	if(DCMI_GetITStatus(DCMI_IT_FRAME)==SET)//����һ֡ͼ��
	{
        DCMI_Stop();
        
        if(Trace(&condition, &result))
        {
            struct euler_angle euler;
            int camera_x, camera_y;
            int plate_x, plate_y;
            uint16_t pixel = 0xF800;
            
            dmp_get_eulerangle(&euler);
            camera_x = (int)((int)(result.x - ORIGIN_XPIXEL) * XPIXEL_TO_MM);
            camera_y = (int)((int)(result.y - ORIGIN_YPIXEL) * YPIXEL_TO_MM);
            plate_x = camera_x / cos(euler.roll * ANGLE_TO_RADIAN) 
                    + (- OFFSET_HEIGHT * tan(euler.roll * ANGLE_TO_RADIAN));
            plate_y = camera_y / cos(euler.pitch * ANGLE_TO_RADIAN) 
                    + (OFFSET_HEIGHT * tan(euler.pitch * ANGLE_TO_RADIAN));
            rt_kprintf("pitch: %d�� roll: %d�� ", (int)euler.pitch, (int)euler.roll);
            rt_kprintf("x: %dmm y: %dmm ", plate_x, plate_y);
            rt_kprintf("tick: %d\n", rt_tick_get());
            ili9486l_set_pixel(&pixel, result.x, result.y);
            ili9486l_set_pixel(&pixel, result.x - 1, result.y);
            ili9486l_set_pixel(&pixel, result.x + 1, result.y);
            ili9486l_set_pixel(&pixel, result.x, result.y - 1);
            ili9486l_set_pixel(&pixel, result.x, result.y + 1);
            ili9486l_draw_hline(&pixel, 
                                result.x - result.w / 2, 
                                result.x + result.w / 2, 
                                result.y - result.h / 2);
            ili9486l_draw_hline(&pixel, 
                                result.x - result.w / 2, 
                                result.x + result.w / 2, 
                                result.y + result.h / 2);
            ili9486l_draw_vline(&pixel, 
                                result.x - result.w / 2, 
                                result.y - result.h / 2, 
                                result.y + result.h / 2);
            ili9486l_draw_vline(&pixel, 
                                result.x + result.w / 2, 
                                result.y - result.h / 2, 
                                result.y + result.h / 2);                    
        }
        ili9486l_set_display_window(0, 0, 480, 320);
        *((volatile rt_uint16_t *) 0x60000000) = 0x2C;
        
		DCMI_ClearITPendingBit(DCMI_IT_FRAME);//���֡�ж�
        DCMI_Start();
	}
}

//DCMI��DMA����
void DCMI_DMA_Init(rt_ubase_t DMA_Memory0BaseAddr,rt_uint16_t DMA_BufferSize,rt_size_t DMA_MemoryDataSize)
{
	DMA_InitTypeDef DMA_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//ʹ��DMA2ʱ��
	DMA_DeInit(DMA2_Stream1);
	while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);//�ȴ�DMA2_Stream1������
	
	DMA_InitStructure.DMA_BufferSize=DMA_BufferSize;//���ݴ�����
	DMA_InitStructure.DMA_Channel=DMA_Channel_1;//DMA2ͨ��1
	DMA_InitStructure.DMA_DIR=DMA_DIR_PeripheralToMemory;//���赽������ģʽ
	DMA_InitStructure.DMA_FIFOMode=DMA_FIFOMode_Enable;//FIFOģʽ 
	DMA_InitStructure.DMA_FIFOThreshold=DMA_FIFOThreshold_Full;//ʹ��ȫFIFO
	DMA_InitStructure.DMA_Memory0BaseAddr=DMA_Memory0BaseAddr;//��������ַ
	DMA_InitStructure.DMA_MemoryBurst=DMA_MemoryBurst_Single;//��洢��ͻ�����δ���
	DMA_InitStructure.DMA_MemoryDataSize=DMA_MemoryDataSize;//���������ݳ���
	DMA_InitStructure.DMA_MemoryInc=DMA_MemoryInc_Disable;//�洢��������ģʽ
	DMA_InitStructure.DMA_Mode=DMA_Mode_Circular;//ʹ��ѭ��ģʽ
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr=(uint32_t)&(DCMI->DR);//�����ַΪ:DCMI->DR
	DMA_InitStructure.DMA_PeripheralBurst=DMA_PeripheralBurst_Single;//����ͻ�����δ�
	DMA_InitStructure.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Word;//�������ݳ���:32λ
	DMA_InitStructure.DMA_PeripheralInc=DMA_PeripheralInc_Disable;//���������ģʽ
	DMA_InitStructure.DMA_Priority=DMA_Priority_VeryHigh;//�����ȼ�
	DMA_Init(DMA2_Stream1,&DMA_InitStructure);//���ô�DMA2_Stream1
}
//DCMI,��������
void DCMI_Start(void)
{
    DMA_Cmd(DMA2_Stream1, ENABLE);//����DMA2,Stream1 
    DCMI_CaptureCmd(ENABLE);//DCMI����ʹ��  
}
//DCMI,�رմ���
void DCMI_Stop(void)
{ 
    DCMI_CaptureCmd(DISABLE);//DCMI����ʹ�ر�
    while(DCMI->CR&0X01);		//�ȴ��������
    DMA_Cmd(DMA2_Stream1,DISABLE);//�ر�DMA2,Stream1
}
