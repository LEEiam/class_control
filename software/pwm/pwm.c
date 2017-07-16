#include <rtthread.h>
#include "board.h"
#include "finsh.h"

volatile int16_t Motor_freq=25000;       //�������Ƶ��  

#define MAX_PWM_DUTY 80

static void Timer_Gpio_Init(void);      //��ʱ�����ų�ʼ��
static void Timer_Pwm_Out_Mode(void);   //Ƶ�����巢��
static void Timer_Encode_Mode(void);     //��������ʼ��
static void Zero_Clear(void);
static void Timer4_CH1to4_Config(void);

int stm32_hw_timer_init(void);   

int PWM1_Set(uint8_t num)
{
	if(num<=MAX_PWM_DUTY)
		TIM_SetCompare1(TIM4,(int)((1000000/Motor_freq-1)*(num*0.01)));
	else rt_kprintf("input invalid!");
	return 0;
}
int PWM2_Set(uint8_t num)
{
	if(num<=MAX_PWM_DUTY)
		TIM_SetCompare2(TIM4,(int)((1000000/Motor_freq-1)*(num*0.01)));
	else rt_kprintf("input invalid!");
	return 0;
}
int PWM3_Set(uint8_t num)
{
	if(num<=MAX_PWM_DUTY)
		TIM_SetCompare3(TIM4,(int)((1000000/Motor_freq-1)*(num*0.01)));
	else rt_kprintf("input invalid!");
	return 0;
}
int PWM4_Set(uint8_t num)
{
	if(num<=MAX_PWM_DUTY)
		TIM_SetCompare4(TIM4,(int)((1000000/Motor_freq-1)*(num*0.01)));
	else rt_kprintf("input invalid!");
	return 0;
}
  

INIT_BOARD_EXPORT(stm32_hw_timer_init);    
FINSH_FUNCTION_EXPORT_ALIAS(PWM1_Set,pwm1,set pwm1 duty percent);
FINSH_FUNCTION_EXPORT_ALIAS(PWM2_Set,pwm2,set pwm2 duty percent);
FINSH_FUNCTION_EXPORT_ALIAS(PWM3_Set,pwm3,set pwm3 duty percent);
FINSH_FUNCTION_EXPORT_ALIAS(PWM4_Set,pwm4,set pwm3 duty percent);

int stm32_hw_timer_init(void)
{
	Timer_Gpio_Init();
	Timer4_CH1to4_Config();
//	Timer_Pwm_Out_Mode();
//	Timer_Encode_Mode();
//	Zero_Clear();
	
	return 0;
}

/*�жϷ�����*/

void TIM8_TRG_COM_TIM14_IRQHandler(void)
{	
		/* enter interrupt */
    rt_interrupt_enter();
	
	  GPIO_ToggleBits(GPIOA,GPIO_Pin_7);

		
	
	  TIM_ClearFlag(TIM14,TIM_FLAG_Update);
	
		/* leave interrupt */
    rt_interrupt_leave();
}

void EXTI0_IRQHandler(void)
{
	/* enter interrupt */
    rt_interrupt_enter();

	GPIO_ToggleBits(GPIOD,GPIO_Pin_12);
	
	TIM1->CNT=0;
	EXTI_ClearITPendingBit(EXTI_Line0);
 

	/* leave interrupt */
    rt_interrupt_leave();
}

static void Timer_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_TIM1);      //TIM1_CH2--->PE11
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_TIM1);       //TIM1_CH1 --->PE9
	
	//TIM14_init
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_7;   										//PWM  TIM14
  GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;       						//    <--��ƽ��תҪ�޸�Ϊ���ģʽ
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM14);  
	
	 //LED_init
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12;                  
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	
	//TIM4_CH1~4_init
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;   
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_TIM4);    //PD12 --> CH1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4);    //PD13 --> CH2
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_TIM4);    //PD14 --> CH3
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_TIM4);    //PD15 --> CH4
	
	///////������////////
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
//	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;                //��������
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
//	
//	GPIOE->ODR|=((1<<9)|(1<<11));
	
	GPIO_Init(GPIOE,&GPIO_InitStruct);                    //PE11_init
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;                 //PE9_init
	GPIO_Init(GPIOE,&GPIO_InitStruct);       
	/////////////////////////

  GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;                 //PE7_init    -->�������
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	GPIO_Init(GPIOB,&GPIO_InitStruct);       
	
}
/*
�ⲿ�ж�����ע�����
1.��SYSCFGʱ��Դ�������Ž����ⲿ�ж�
2.�ⲿ�ж������Ƿ���ģ��磺PA0��PB0...��ӦEXTI_line0
3.PB2����ΪBOOT1���ţ����ʺ���Ϊ��������
4.ϵͳʱ��ԴSYSCFG�����Ļ����޷���EXTICR�Ĵ������в�����Ĭ����PA�˿�Ϊ�ж�����(�Ĵ���Ĭ��ֵΪ0)
*/


static void Zero_Clear(void)
{
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);   //ʹ��ϵͳʱ��
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB,EXTI_PinSource0);   //Selects the PE7 used as EXIT Line;
//	SYSCFG->EXTICR[0]=1<<8;
	
	EXTI_InitStruct.EXTI_Line=EXTI_Line0;   
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt; 
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	
	EXTI_Init(&EXTI_InitStruct);
		
	NVIC_InitStruct.NVIC_IRQChannel=EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	
	
}
/*
  //�ж����ŵ�ƽ��ת����PWM
static void Timer_Pwm_Out_Mode2(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
//	TIM_OCInitTypeDef TIM_OCInitStruct;
	
	NVIC_InitStruct.NVIC_IRQChannel=TIM8_TRG_COM_TIM14_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14,ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;		 //���ö�ʱ��Ƶ��CK_INTƵ�������ʱ��Ƶ�ʱ�,һ�㲻��Ҫ
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler=83; 											//��Ƶ������ȷ��
	TIM_TimeBaseInitStruct.TIM_Period=(1000000/Motor_freq-1)/2;       //1K
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;   						//�ظ�
	
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseInitStruct);
	
	TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);                    //ʹ���ж�
	
	TIM_Cmd(TIM14,ENABLE);
}*/

   //Ӳ������PWM
static void Timer_Pwm_Out_Mode(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14,ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;		 //���ö�ʱ��Ƶ��CK_INTƵ�������ʱ��Ƶ�ʱ�,һ�㲻��Ҫ
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler=83; 											//��Ƶ������ȷ��
	TIM_TimeBaseInitStruct.TIM_Period=1000000/Motor_freq-1;       //1K
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;   						//�ظ�
	
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseInitStruct);
	
	
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;   //ָ����Ч��ƽ
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
   TIM_OCInitStruct.TIM_Pulse=(1000000/Motor_freq-1)/2;
//	TIM_OCInitStruct.TIM_OCIdleState=TIM_OCIdleState_Reset;   //�ƶ�����״̬��TIM1/8��Ч
//	TIM_OCInitStruct.TIM_OCNIdleState=TIM_OCNIdleState_Set;   //�ƶ�����״̬��TIM1/8��Ч
//	TIM_OCInitStruct.TIM_OCNPolarity=TIM_OCNPolarity_High;   
//	TIM_OCInitStruct.TIM_OutputNState=TIM_OutputNState_Disable ; //TIM1/8��Ч
//	
	TIM_OC1Init(TIM14,&TIM_OCInitStruct);
	
	TIM_OC1PreloadConfig(TIM14,TIM_OCPreload_Enable);    //ʱ��CCR�Ƚ�ͨ�����ص㣡����
	TIM_ARRPreloadConfig(TIM14,ENABLE);
	
	TIM_Cmd(TIM14,ENABLE);

}
static void Timer4_CH1to4_Config(void)
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
		TIM_OCInitTypeDef TIM_OCInitStruct;
		

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
		
		TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;		 //���ö�ʱ��Ƶ��CK_INTƵ�������ʱ��Ƶ�ʱ�,һ�㲻��Ҫ
		TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
		TIM_TimeBaseInitStruct.TIM_Prescaler=83; 											//��Ƶ������ȷ��
		TIM_TimeBaseInitStruct.TIM_Period=1000000/Motor_freq-1;       //1K
		TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;   						//�ظ�
		
		TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
		
		TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;
		TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;   //ָ����Ч��ƽ
		TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
		TIM_OCInitStruct.TIM_Pulse=(1000000/Motor_freq-1)*0;

		TIM_OC1Init(TIM4,&TIM_OCInitStruct);
		TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);    //ʱ��CCR�Ƚ�ͨ�����ص㣡����
		TIM_OC2Init(TIM4,&TIM_OCInitStruct);
		TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);    //ʱ��CCR�Ƚ�ͨ�����ص㣡����
		TIM_OC3Init(TIM4,&TIM_OCInitStruct);
		TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);    //ʱ��CCR�Ƚ�ͨ�����ص㣡����
		TIM_OC4Init(TIM4,&TIM_OCInitStruct);
		TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);    //ʱ��CCR�Ƚ�ͨ�����ص㣡����
		
		TIM_ARRPreloadConfig(TIM4,ENABLE);
		
		TIM_Cmd(TIM4,ENABLE);
}

static void Timer_Encode_Mode(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_ICInitTypeDef TIM_ICInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;		 //���ö�ʱ��Ƶ��CK_INTƵ�������ʱ��Ƶ�ʱ�,һ�㲻��Ҫ
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler=9; 											//��Ƶ������ȷ��
	TIM_TimeBaseInitStruct.TIM_Period=40000;       //1K
//	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;   						//�ظ�
	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
	
	TIM_ICInitStruct.TIM_Channel=TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter=0;      
	TIM_ICInitStruct.TIM_ICPolarity=TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection=TIM_ICSelection_DirectTI;
	
	TIM_ICInit(TIM1,&TIM_ICInitStruct);              //��ʼ������CH1ͨ��
	
	TIM_ICInitStruct.TIM_Channel=TIM_Channel_2;      //��ʼ������CH2ͨ��
	
	TIM_ICInit(TIM1,&TIM_ICInitStruct);
	TIM1->SMCR |= 3<<0;	
//	TIM_EncoderInterfaceConfig(TIM1,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);
	
	TIM_Cmd(TIM1,ENABLE);
}