#include "board.h"
#include "sccb.h"

//IO操作函数	 
#define SCCB_SCL    		PEout(2)//写OV_SCL
#define SCCB_SDA    		PEout(3)//写OV_SDA
#define SCCB_READ_SDA       PBin(3) //读OV_SDA
#define SCCB_OV2640_WID     0X60    //OV2640的写ID
#define SCCB_OV2640_RID     0X61    //OV2640的读ID

//SCCB端口配置
void SCCB_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOE时钟
	//OV_SCL OV_SDA 设置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;//PE2,3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;//2MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化
}

//SCCB_SDA配置为输出
void SCCB_SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PE3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化
}

//SCCB_SDA配置为输入
void SCCB_SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PE3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化
}

//SCCB起始信号
//当时钟为高的时候,数据线的高到低,为SCCB起始信号
//在激活状态下,SDA和SCL均为低电平
void SCCB_Start(void)
{
    SCCB_SDA = 1;//数据线高电平	   
    SCCB_SCL = 1;//在时钟线高的时候
    rt_hw_us_delay(2);
    SCCB_SDA = 0;//数据线由高至低
    rt_hw_us_delay(2);
    SCCB_SCL = 0;//数据线恢复低电平，单操作函数必要	  
}

//SCCB停止信号
//当时钟为高的时候,数据线的低到高,为SCCB停止信号
//空闲状况下,SDA,SCL均为高电平
void SCCB_Stop(void)
{
    SCCB_SDA=0;//数据线低电平
    rt_hw_us_delay(2);
    SCCB_SCL=1;//在时钟高的时候	
    rt_hw_us_delay(2);
    SCCB_SDA=1;//数据线由低至高	
} 

//向SCCB写一个字节
int SCCB_WByte(unsigned char DATA)
{
	unsigned char i;
	//循环8次写一个字节
	for(i=0;i<8;i++)
	{
		SCCB_SDA = ((0X80)&DATA?1:0);//将DATA的最高位写入SCCB_SDA
		DATA <<= 1;//DATA左移1位
		rt_hw_us_delay(2);
		SCCB_SCL = 1;//SCCB_SCL上升沿写入数据
		rt_hw_us_delay(2);
		SCCB_SCL = 0;
	}
	//读取 ACK bit
	SCCB_SDA_IN();//SCCB_SDA设置为输入
	rt_hw_us_delay(2);
	SCCB_SCL = 1;//SCCB_SCL上升沿读取应答信号
	rt_hw_us_delay(2);
	if(SCCB_READ_SDA)//如果NO ACK
	{
		SCCB_SCL = 0;
		SCCB_SDA_OUT();//SCCB_SDA设置为输出
		return 1;//表明写入失败
	}
	else//如果ACK
	{
		SCCB_SCL = 0;
		SCCB_SDA_OUT();//SCCB_SDA设置为输出
		return 0;//表明写入成功
	}
}

//从SCCB读一个字节
unsigned char SCCB_Rbyte(void)
{
	unsigned char i,temp=0;
	SCCB_SDA_IN();//SCCB_SDA设置为输入
	//循环8次读一个字节
	for(i=0;i<8;i++)
	{
		rt_hw_us_delay(2);
		SCCB_SCL = 1;//SCCB_SCL上升沿读取数据
		rt_hw_us_delay(2);
		temp <<= 1;
		temp |= SCCB_READ_SDA;//读取1位数据
		SCCB_SCL = 0;
	}
	SCCB_SDA_OUT();//SCCB_SDA设置为输出
	return temp;
}

//SCCB写寄存器
//RESULT 0:写入成功 1:写入失败
int SCCB_WReg(unsigned char RegID,unsigned char DATA)
{
	unsigned char RESULT=0;
	SCCB_Start();//SCCB启动传输
	RESULT |= SCCB_WByte(SCCB_OV2640_WID);//OV2640的写地址
	RESULT |= SCCB_WByte(RegID);//写入寄存器地址
	RESULT |= SCCB_WByte(DATA);//向寄存器写数据
	SCCB_Stop();//SCCB结束传输
	return RESULT;
}

//SCCB读寄存器
unsigned char SCCB_RReg(unsigned char RegID)
{
	unsigned char temp;
	SCCB_Start();//SCCB启动传输
	SCCB_WByte(SCCB_OV2640_WID);//OV2640的写地址
	SCCB_WByte(RegID);//写入寄存器地址
	SCCB_Stop();//SCCB结束传输
	SCCB_Start();//SCCB启动传输
	SCCB_WByte(SCCB_OV2640_RID);//OV2640的读地址
	temp = SCCB_Rbyte();//从寄存器读数据
	SCCB_NOACK();//产生NO ACK信号
	SCCB_Stop();//SCCB结束传输
	return temp;//返回读取的值
}

//产生NO ACK信号
//第9bit输出高电平
void SCCB_NOACK(void)
{
    SCCB_SDA=1;//数据线高电平
    rt_hw_us_delay(2);
    SCCB_SCL=1;//时钟线高电平
    rt_hw_us_delay(2);
    SCCB_SCL=0;//时钟线低电平
}
