#include "board.h"
#include "sccb.h"

//IO��������	 
#define SCCB_SCL    		PEout(2)//дOV_SCL
#define SCCB_SDA    		PEout(3)//дOV_SDA
#define SCCB_READ_SDA       PBin(3) //��OV_SDA
#define SCCB_OV2640_WID     0X60    //OV2640��дID
#define SCCB_OV2640_RID     0X61    //OV2640�Ķ�ID

//SCCB�˿�����
void SCCB_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOEʱ��
	//OV_SCL OV_SDA ����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;//PE2,3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;//2MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��
}

//SCCB_SDA����Ϊ���
void SCCB_SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PE3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��
}

//SCCB_SDA����Ϊ����
void SCCB_SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PE3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��
}

//SCCB��ʼ�ź�
//��ʱ��Ϊ�ߵ�ʱ��,�����ߵĸߵ���,ΪSCCB��ʼ�ź�
//�ڼ���״̬��,SDA��SCL��Ϊ�͵�ƽ
void SCCB_Start(void)
{
    SCCB_SDA = 1;//�����߸ߵ�ƽ	   
    SCCB_SCL = 1;//��ʱ���߸ߵ�ʱ��
    rt_hw_us_delay(2);
    SCCB_SDA = 0;//�������ɸ�����
    rt_hw_us_delay(2);
    SCCB_SCL = 0;//�����߻ָ��͵�ƽ��������������Ҫ	  
}

//SCCBֹͣ�ź�
//��ʱ��Ϊ�ߵ�ʱ��,�����ߵĵ͵���,ΪSCCBֹͣ�ź�
//����״����,SDA,SCL��Ϊ�ߵ�ƽ
void SCCB_Stop(void)
{
    SCCB_SDA=0;//�����ߵ͵�ƽ
    rt_hw_us_delay(2);
    SCCB_SCL=1;//��ʱ�Ӹߵ�ʱ��	
    rt_hw_us_delay(2);
    SCCB_SDA=1;//�������ɵ�����	
} 

//��SCCBдһ���ֽ�
int SCCB_WByte(unsigned char DATA)
{
	unsigned char i;
	//ѭ��8��дһ���ֽ�
	for(i=0;i<8;i++)
	{
		SCCB_SDA = ((0X80)&DATA?1:0);//��DATA�����λд��SCCB_SDA
		DATA <<= 1;//DATA����1λ
		rt_hw_us_delay(2);
		SCCB_SCL = 1;//SCCB_SCL������д������
		rt_hw_us_delay(2);
		SCCB_SCL = 0;
	}
	//��ȡ ACK bit
	SCCB_SDA_IN();//SCCB_SDA����Ϊ����
	rt_hw_us_delay(2);
	SCCB_SCL = 1;//SCCB_SCL�����ض�ȡӦ���ź�
	rt_hw_us_delay(2);
	if(SCCB_READ_SDA)//���NO ACK
	{
		SCCB_SCL = 0;
		SCCB_SDA_OUT();//SCCB_SDA����Ϊ���
		return 1;//����д��ʧ��
	}
	else//���ACK
	{
		SCCB_SCL = 0;
		SCCB_SDA_OUT();//SCCB_SDA����Ϊ���
		return 0;//����д��ɹ�
	}
}

//��SCCB��һ���ֽ�
unsigned char SCCB_Rbyte(void)
{
	unsigned char i,temp=0;
	SCCB_SDA_IN();//SCCB_SDA����Ϊ����
	//ѭ��8�ζ�һ���ֽ�
	for(i=0;i<8;i++)
	{
		rt_hw_us_delay(2);
		SCCB_SCL = 1;//SCCB_SCL�����ض�ȡ����
		rt_hw_us_delay(2);
		temp <<= 1;
		temp |= SCCB_READ_SDA;//��ȡ1λ����
		SCCB_SCL = 0;
	}
	SCCB_SDA_OUT();//SCCB_SDA����Ϊ���
	return temp;
}

//SCCBд�Ĵ���
//RESULT 0:д��ɹ� 1:д��ʧ��
int SCCB_WReg(unsigned char RegID,unsigned char DATA)
{
	unsigned char RESULT=0;
	SCCB_Start();//SCCB��������
	RESULT |= SCCB_WByte(SCCB_OV2640_WID);//OV2640��д��ַ
	RESULT |= SCCB_WByte(RegID);//д��Ĵ�����ַ
	RESULT |= SCCB_WByte(DATA);//��Ĵ���д����
	SCCB_Stop();//SCCB��������
	return RESULT;
}

//SCCB���Ĵ���
unsigned char SCCB_RReg(unsigned char RegID)
{
	unsigned char temp;
	SCCB_Start();//SCCB��������
	SCCB_WByte(SCCB_OV2640_WID);//OV2640��д��ַ
	SCCB_WByte(RegID);//д��Ĵ�����ַ
	SCCB_Stop();//SCCB��������
	SCCB_Start();//SCCB��������
	SCCB_WByte(SCCB_OV2640_RID);//OV2640�Ķ���ַ
	temp = SCCB_Rbyte();//�ӼĴ���������
	SCCB_NOACK();//����NO ACK�ź�
	SCCB_Stop();//SCCB��������
	return temp;//���ض�ȡ��ֵ
}

//����NO ACK�ź�
//��9bit����ߵ�ƽ
void SCCB_NOACK(void)
{
    SCCB_SDA=1;//�����߸ߵ�ƽ
    rt_hw_us_delay(2);
    SCCB_SCL=1;//ʱ���߸ߵ�ƽ
    rt_hw_us_delay(2);
    SCCB_SCL=0;//ʱ���ߵ͵�ƽ
}
